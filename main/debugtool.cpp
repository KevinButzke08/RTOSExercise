#include "debugtool.h"
#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/ringbuf.h"
#include <cstdint>
#include <cstring>

RingbufHandle_t rb;
static bool trace_enabled = true;

void debugtool_init() {
  MetaTask mTask = {.num_tasks = 2, .ticksToRun = 500};
  rb = xRingbufferCreate(30000, RINGBUF_TYPE_NOSPLIT);
  xTaskCreate(debugtool_task, "debugtool_task", 4096, (void *)&mTask,
              configMAX_PRIORITIES - 1, NULL);
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
    if (_currMessage == NULL)
      break;
    MESSAGE_TYPE type = *(MESSAGE_TYPE *)_currMessage;
    switch (type) {
    case MESSAGE_TYPE_QUEUE:
      print_logmessage((QueueMessage *)_currMessage);
      break;
    case MESSAGE_TYPE_TICK:
      print_incrementTickMessage((IncrementTickMessage *)_currMessage);
      break;
    case MESSAGE_TYPE_TASK:
      print_taskMessage((TaskMessage *)_currMessage);
      break;
    case MESSAGE_TYPE_TASK_DELAY:
      print_taskDelayMessage((TaskDelayMessage *)_currMessage);
      break;
    }
  }
  ESP_LOGI("DEBUGTOOL", "END OF TRACE");
  while (1) {
  }
}
/**
 * Helper functions to translate the enum events to Strings
 */
const char *queue_event_to_string(int event) {
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
    return "UNKNOWN_QUEUE_EVENT";
  }
}

const char *task_event_to_string(int event) {
  switch (event) {
  case TASK_EVENT_CREATE:
    return "TASK_CREATE";
  case TASK_EVENT_CREATE_FAILED:
    return "TASK_CREATE_FAILED";
  case TASK_EVENT_DELETE:
    return "TASK_DELETE";
  case TASK_EVENT_DELAY:
    return "TASK_DELAY";
  case TASK_EVENT_DELAY_UNTIL:
    return "TASK_DELAY_UNTIL";
  case TASK_EVENT_SWITCHED_IN:
    return "TASK_SWITCHED_IN";
  case TASK_EVENT_SWITCHED_OUT:
    return "TASK_SWITCHED_OUT";
  default:
    return "UNKNOWN_TASK_EVENT";
  }
}

/**
 * Prints out a `LogMessage` struct
 */
void print_logmessage(QueueMessage *_lm) {
  QueueMessage lm = *_lm;
  QueueHandle_t queue_handle = (QueueHandle_t)lm.generic_data;
  ESP_LOGI("DEBUG",
           "Event: %s, Task: %s, Tick: %lu, Timestamp: %ld, Queue Handle: %X",
           queue_event_to_string(lm.event), lm.taskname, lm.tick, lm.timestamp,
           queue_handle);
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
/**
 * Prints out a `TaskMessage` struct
 */
void print_taskMessage(TaskMessage *_tm) {
  TaskMessage tm = *_tm;
  ESP_LOGI("DEBUG", "Event: %s, Tick: %lu, Timestamp: %ld, TaskHandle: %s, ",
           task_event_to_string(tm.event), tm.tick, tm.timestamp,
           tm.taskname);
}
/**
 * Prints out a `TaskDelayMessage` struct
 */
void print_taskDelayMessage(TaskDelayMessage *_tdm) {
  TaskDelayMessage tdm = *_tdm;
  ESP_LOGI("DEBUG",
           "Event: %s, Tick: %lu, TicksToDelay: %lu, Timestamp: %ld, "
           "TaskHandle: %s, ",
           task_event_to_string(tdm.event), tdm.tick, tdm.tickstodelay,
           tdm.timestamp, tdm.taskname);
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
  const char *name = pcTaskGetName(curr_task);

  QueueMessage lm = {.event = e,
                   .tick = xTaskGetTickCount(),
                   .timestamp = (uint32_t)esp_timer_get_time(),
                   .taskhandle = curr_task,
                   .generic_data = (void *)_pxQueue,
                   .taskname = ""};
  // Copy task name safely
  size_t n = strnlen(name ? name : "", configMAX_TASK_NAME_LEN);
  memcpy(lm.taskname, name ? name : "", n);
  lm.taskname[n] = '\0';

  xRingbufferSend(rb, &lm, sizeof(QueueMessage), 0);
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
void tracetask_function(TASK_EVENT event, void *xTask) {
  if (rb == NULL || trace_enabled == false) {
    return;
  }

  TaskMessage tm = {.event = event,
                    .tick = xTaskGetTickCount(),
                    .timestamp = (uint32_t)esp_timer_get_time(),
                    .taskidentifier = (TaskHandle_t)xTask};
  if ((event == TASK_EVENT_SWITCHED_IN) || (event == TASK_EVENT_SWITCHED_OUT)) {
    // pxCurrentTCB = xTaskGetCurrentTaskHandle(), but pxCurrentTCB not defined
    // in this scope
    tm.taskidentifier = xTaskGetCurrentTaskHandle();
  }
  tm.taskname = pcTaskGetName(tm.taskidentifier);
  xRingbufferSend(rb, &tm, sizeof(TaskMessage), 0);
}

void tracetaskdelay_function(TASK_EVENT event, TickType_t xTicks,
                             int isAbsolute) {
  if (rb == NULL || trace_enabled == false) {
    return;
  }
  TaskHandle_t th = xTaskGetCurrentTaskHandle();
  TickType_t delayTicks = xTicks;
  if (isAbsolute) {
    delayTicks = xTicks - xTaskGetTickCount();
  }
  TaskDelayMessage tdm = {.event = event,
                          .tick = xTaskGetTickCount(),
                          .tickstodelay = delayTicks,
                          .timestamp = (uint32_t)esp_timer_get_time(),
                          .taskidentifier = th};
  tdm.taskname = pcTaskGetName(tdm.taskidentifier);
  xRingbufferSend(rb, &tdm, sizeof(TaskDelayMessage), 0);
}

#ifdef __cplusplus
}
#endif
