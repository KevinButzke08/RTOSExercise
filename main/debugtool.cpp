#include "debugtool.h"
#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/ringbuf.h"
#include <cstdint>

RingbufHandle_t rb;

void debugtool_init() { rb = xRingbufferCreate(10000, RINGBUF_TYPE_NOSPLIT); }

/**
 * Prints the collected trace messages after a defined timespan
 */
void debugtool_task(void *pvParameters) {
  MetaTask mTask = *(MetaTask *)pvParameters;
  vTaskDelay(mTask.ticksToRun);
  void *_currMessage;
  LogMessage currMessage;
  size_t recv_size;
  while (1) {
    _currMessage = xRingbufferReceive(rb, &recv_size, 0);
    if (recv_size != sizeof(LogMessage) || _currMessage == NULL)
      break;
    print_logmessage((LogMessage*)_currMessage);
  }
  while (1) {
    vTaskDelay(10000);
  }
}
/**
 * Helper function to translate the enum events to Strings
 */
const char* event_to_string(int event) {
    switch (event) {
        case QUEUE_EVENT_RECEIVE: return "RECEIVE";
        case QUEUE_EVENT_RECEIVE_FAILED: return "RECEIVE_FAILED";
        default: return "UNKNOWN_EVENT";
    }
}

/**
 * Prints out a `LogMessage` struct
 */
void print_logmessage(LogMessage *_lm) {
  LogMessage lm = *_lm;
  TaskStatus_t xTaskDetails;
  vTaskGetInfo(lm.taskhandle, &xTaskDetails,
               pdTRUE,    // Include the high water mark in xTaskDetails.
               eInvalid); // Include the task state in xTaskDetails.
  
    QueueHandle_t queue_handle = (QueueHandle_t)lm.generic_data;
    ESP_LOGI("DEBUG",
             "Event: %s, Task: %s, Tick: %lu, Timestamp: %ld, Queue "
             "Handle: %X",
             event_to_string(lm.event),xTaskDetails.pcTaskName, lm.tick, lm.timestamp, queue_handle);
}

/*********DEFINE TRACE FUNCTIONS HERE******************/
#ifdef __cplusplus
extern "C" {
#endif

void tracequeue_function(QUEUE_EVENT e, void *pxQueue) {
  QueueHandle_t _pxQueue = (QueueHandle_t)pxQueue;
  TaskHandle_t curr_task = xTaskGetCurrentTaskHandle();
  switch (e) {
  case QUEUE_EVENT_RECEIVE: {
    LogMessage lm = {.event = e,
                     .tick = xTaskGetTickCount(),
                     .timestamp = (uint32_t)esp_timer_get_time(),
                     .taskhandle = curr_task,
                     .generic_data = (char *)_pxQueue};
    xRingbufferSend(rb, &lm, sizeof(LogMessage), 0);
    break;
  }
  case QUEUE_EVENT_RECEIVE_FAILED:
    break;
  }
}

#ifdef __cplusplus
}
#endif
