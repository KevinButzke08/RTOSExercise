#include "esp_timer.h"
#include "freertos/ringbuf.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "debugtool.h"

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


extern "C" void app_main() {
  ESP_LOGI("app_main", "Starting scheduler from app_main()");
  queue = xQueueCreate(10, sizeof(Message));
  debugtool_init();
  xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 5, NULL);
  xTaskCreate(sender_task, "sender_task", 4096, (void *)100, 5,
              NULL);
  vTaskStartScheduler();
  /* vTaskStartScheduler is blocking - this should never be reached */
  ESP_LOGE("app_main", "insufficient RAM! aborting");
  abort();
}
