#include "esp_timer.h"
#include "freertos/ringbuf.h"
#include <Display.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TICKS_PER_MS 1000
#define TAG "RTOS"

typedef struct {
  char message[50];
} Message;

QueueHandle_t queue;

void sender_task(void *pvParameters) {
  int period = (uint32_t)pvParameters;
  TickType_t t = xTaskGetTickCount();
  Message message;
  sprintf(message.message, "Sender with %d period", period);
  while (1) {
    xQueueSendToBack(queue, &message, 100);
    vTaskDelayUntil(&t, period);
  }
}

void receiver_task(void *pvParameters) {
  Message message;
  while (1) {
    if (xQueueReceive(queue, &message, portMAX_DELAY)) {
      ESP_LOGI(TAG, "%s", message.message);
    }
  }
}

typedef struct {
  size_t num_tasks;
  TaskHandle_t *tasks;
  TickType_t ticksToRun;
} MetaTask;

RingbufHandle_t rb;

typedef struct {
  QUEUE_EVENT e;
  TickType_t tick;
  int64_t timestamp;
} LogMessage;

void tracequeue_function(QUEUE_EVENT e, void *pxQueue) {
  QueueHandle_t _pxQueue = (QueueHandle_t)pxQueue;
  switch (e) {
  case QUEUE_EVENT_RECEIVE: {
    LogMessage lm = {
        .e = e, .tick = xTaskGetTickCount(), .timestamp = esp_timer_get_time()};
    xRingbufferSend(rb, &lm, sizeof(LogMessage), 0);
    break;
  }
  case QUEUE_EVENT_RECEIVE_FAILED:
    break;
  }
}

void print_logmessage(LogMessage lm) {
  switch (lm.e) {
  case QUEUE_EVENT_RECEIVE: {
    ESP_LOGI("DEBUG", "Event: RECEIVE, Tick: %ul, Timestamp: %ul", lm.tick,
             lm.timestamp);
    break;
  }
  case QUEUE_EVENT_RECEIVE_FAILED:
    break;
  }
}

void destroy_task(void *pvParameters) {
  MetaTask mTask = *(MetaTask *)pvParameters;
  vTaskDelay(mTask.ticksToRun);
  // for (int i = 0; i < mTask.num_tasks; i++) {
  //     vTaskDelete(mTask.tasks[i]);
  // }
  // TODO: Print log buffea
  void *_currMessage;
  LogMessage currMessage;
  size_t recv_size;
  while (1) {
    _currMessage = xRingbufferReceive(rb, &recv_size, 0);
    if (recv_size != sizeof(LogMessage) || _currMessage == NULL)
      break;
    currMessage = *(LogMessage *)_currMessage;
    print_logmessage(currMessage);
  }
  while (1) {
    vTaskDelay(10000);
  }
}

extern "C" void app_main() {
  ESP_LOGI("app_main", "Starting scheduler from app_main()");
  queue = xQueueCreate(10, sizeof(Message));
  rb = xRingbufferCreate(10000, RINGBUF_TYPE_NOSPLIT);
  TaskHandle_t task_handles[10];
  MetaTask mTask = {.num_tasks = 2, .tasks = task_handles, .ticksToRun = 3000};
  xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 5, &task_handles[0]);
  xTaskCreate(sender_task, "sender_task", 4096, (void *)100, 5,
              &task_handles[1]);
  xTaskCreate(destroy_task, "destroy_task", 4096, (void *)&mTask, 10, NULL);
  vTaskStartScheduler();
  /* vTaskStartScheduler is blocking - this should never be reached */
  ESP_LOGE("app_main", "insufficient RAM! aborting");
  abort();
}
