#include "debugtool.h"
#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/ringbuf.h"
#include <cstdint>

RingbufHandle_t rb;
static bool trace_enabled = true;

void debugtool_init() {
  MetaTask mTask = {.num_tasks = 2, .ticksToRun = 1000};
  rb = xRingbufferCreate(30000, RINGBUF_TYPE_NOSPLIT);
  xTaskCreate(debugtool_task, "destroy_task", 4096, (void *)&mTask, configMAX_PRIORITIES - 1, NULL);
}

/**
 * Prints the collected trace messages after a defined timespan
 */
void debugtool_task(void *pvParameters) {
  MetaTask mTask = *(MetaTask *)pvParameters;
  TickType_t t = xTaskGetTickCount();
  vTaskDelayUntil(&t, mTask.ticksToRun);
  trace_enabled = false;
  ESP_LOGI("DEBUGTOOL", "Delay of %d ticks ended", mTask.ticksToRun);
  void *_currMessage;
  size_t recv_size;
  while (1) {
    _currMessage = xRingbufferReceive(rb, &recv_size, 0);
    if(_currMessage == NULL) {
      break;
    } else if(recv_size == sizeof(LogMessage)) {
      print_logmessage((LogMessage *)_currMessage);
    } else if (recv_size == sizeof(IncrementTickMessage)) {
      print_incrementTickMessage((IncrementTickMessage *)_currMessage);
    } 
  }
  while (1) {
  }
}
/**
 * Helper function to translate the enum events to Strings
 */
const char *event_to_string(int event) {
  switch (event) {
  case QUEUE_EVENT_RECEIVE:
    return "RECEIVE";
  case QUEUE_EVENT_RECEIVE_FAILED:
    return "RECEIVE_FAILED";
  case QUEUE_EVENT_RECEIVE_FROM_ISR:
    return "RECEIVE_ISR";
  case QUEUE_EVENT_RECEIVE_FROM_ISR_FAILED:
    return "RECEIVE_ISR_FAILED";
  case QUEUE_EVENT_SEND:
    return "SEND";
  case QUEUE_EVENT_SEND_FAILED:
    return "SEND_FAILED";
  case QUEUE_EVENT_SEND_FROM_ISR:
    return "SEND_ISR";
  case QUEUE_EVENT_SEND_FROM_ISR_FAILED:
    return "SEND_ISR_FAILED";
  default:
    return "UNKNOWN_EVENT";
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
           event_to_string(lm.event), xTaskDetails.pcTaskName, lm.tick,
           lm.timestamp, queue_handle);
}
/**
 * Prints out a `IncrementTickMessage` struct
 */
void print_incrementTickMessage(IncrementTickMessage *_im) {
  IncrementTickMessage im = *_im;
  ESP_LOGI("DEBUG",
           "Event: TICK_INCREMENT, Tick: %lu, New_Tick: %lu, Timestamp: %ld, ",
            im.tick, im.new_tick, im.timestamp);
}

/*********DEFINE TRACE FUNCTIONS HERE******************/
#ifdef __cplusplus
extern "C" {
#endif

void tracequeue_function(QUEUE_EVENT e, void *pxQueue) {
  if (rb == NULL || trace_enabled == false) {
    return;
  }
  QueueHandle_t _pxQueue = (QueueHandle_t)pxQueue;
  TaskHandle_t curr_task = xTaskGetCurrentTaskHandle();

  LogMessage lm = {.event = e,
                   .tick = xTaskGetTickCount(),
                   .timestamp = (uint32_t)esp_timer_get_time(),
                   .taskhandle = curr_task,
                   .generic_data = (char *)_pxQueue};
  xRingbufferSend(rb, &lm, sizeof(LogMessage), 0);
}
void tracetick_function(uint32_t xTickCount) {
  if (rb == NULL || trace_enabled == false) {
    return;
  }
  IncrementTickMessage im = {.tick = xTickCount,
                             .new_tick = xTickCount + 1,
                             .timestamp = (uint32_t)esp_timer_get_time()};
  xRingbufferSend(rb, &im, sizeof(IncrementTickMessage), 0);
}

#ifdef __cplusplus
}
#endif
