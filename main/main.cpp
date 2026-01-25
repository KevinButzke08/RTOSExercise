#include "esp_timer.h"
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

typedef struct {
  uint32_t request_id;
} Aperiodic_request;

QueueHandle_t queue;

QueueHandle_t ready_queue;

void hp_task(void *pvParameters) {
  while(1) {
    for(int i = 0; i<200000; i++) {

    }
    vTaskDelay(25);
  }
}

void polling_task(void *pvParameters) {
  Aperiodic_request request;
  TickType_t lastWakeTime = xTaskGetTickCount();
  while (1) {
    if(xQueueReceive(ready_queue, &request, portMAX_DELAY)) {
      ESP_LOGI("APERIODIC:", "Excecuting aperiodic request: %d", request.request_id);
      for(int i=0; i<500; i++) {
      
      }
    }
    vTaskDelayUntil(&lastWakeTime, 500);
  }
}

void aperiodic_request_generator(void *pvParameters) {
  uint32_t id=0;
  while (1) {
    // Time to wait 10ms - 200ms
    TickType_t ticksToWait = pdMS_TO_TICKS(10 + (rand() % 190));
    vTaskDelay(ticksToWait);
    
    Aperiodic_request ap_req;
    ap_req.request_id = ++id;
    xQueueSendToBack(ready_queue, &ap_req , 100);
  }
}
  void lp_task(void *pvParameters) {
    while(1) {
    for(int i = 0; i<10000; i++) {

    }
    vTaskDelay(15);
  }
}

extern "C" void app_main() {
  ESP_LOGI("app_main", "Starting scheduler from app_main()");
  queue = xQueueCreate(10, sizeof(Message));
  ready_queue = xQueueCreate(10, sizeof(Aperiodic_request));
  debugtool_init();
  // xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 7, NULL);
  // xTaskCreate(sender_task, "sender_task", 4096, (void *)100, 5, NULL);
  // xTaskCreate(sender_task, "sender_task2", 4096, (void *)100, 6, NULL);
  xTaskCreate(hp_task, "HP_task", 4096, NULL, 6, NULL);
  xTaskCreate(polling_task, "polling_task", 4096, (void *)1, 5, NULL);
  xTaskCreate(aperiodic_request_generator, "aperiodic_request_generator", 4096, NULL, 4, NULL);
  xTaskCreate(lp_task, "LP_task", 4096, NULL, 3, NULL);
  vTaskStartScheduler();
  /* vTaskStartScheduler is blocking - this should never be reached */
  ESP_LOGE("app_main", "insufficient RAM! aborting");
  abort();
}
