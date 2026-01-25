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
  TaskHandle_t owner;
  QueueHandle_t handle;
  UBaseType_t initial_priority;
} PipSemaphore_t;

BaseType_t xPipTake(PipSemaphore_t *semaphore, TickType_t xTicksToWait) {
  // Priority inheritance ....
  const char *name = pcTaskGetName(xTaskGetCurrentTaskHandle());
  if (semaphore->owner == NULL) {
    ESP_LOGI("SEM", "Taking semaphore, %s", name);
    semaphore->owner = xTaskGetCurrentTaskHandle();
    semaphore->initial_priority = uxTaskPriorityGet(semaphore->owner);
  } else {
    ESP_LOGI("SEM", "Increasing Priority at %lu", (uint32_t)esp_timer_get_time());
    TaskHandle_t curr_task = xTaskGetCurrentTaskHandle();
    UBaseType_t priority = uxTaskPriorityGet(curr_task);
    if (priority > uxTaskPriorityGet(semaphore->owner)) {
      UBaseType_t new_priority = (priority < configMAX_PRIORITIES - 2)
                                     ? priority + 1
                                     : configMAX_PRIORITIES - 1;
      vTaskPrioritySet(semaphore->owner, new_priority);
    }
  }
  return xSemaphoreTake(semaphore->handle, xTicksToWait);
}

BaseType_t xPipGive(PipSemaphore_t *semaphore) {
  const char *name = pcTaskGetName(xTaskGetCurrentTaskHandle());
  ESP_LOGI("SEM", "Giving semaphore, %s", name);
  if (semaphore->owner == xTaskGetCurrentTaskHandle()) {
    vTaskPrioritySet(semaphore->owner, semaphore->initial_priority);
  }
  semaphore->owner = NULL;
  return xSemaphoreGive(semaphore->handle);
}

PipSemaphore_t xPipCreate() {
  PipSemaphore_t semaphore = {.owner = NULL,
                              .handle = xSemaphoreCreateBinary()};
  xSemaphoreGive(semaphore.handle);
  return semaphore;
}

QueueHandle_t queue;
PipSemaphore_t sem;
QueueHandle_t bin_sem;

// Test Task to see if taskDelay shows on Debugger
void test_task(void *pvParameters) {
  int delay = (int)pvParameters;
  vTaskDelay(delay);
  while (1) {
    xPipTake(&sem, portMAX_DELAY);
    // xSemaphoreTake(bin_sem, portMAX_DELAY);
    ESP_LOGI("SEM", "Taking");
    for (int i = 0; i < 100000; i++) {
    }
    xPipGive(&sem);
    // xSemaphoreGive(bin_sem);
    vTaskDelay(10);
  }
}

void inbetween_task(void *pvParameters) {
  vTaskDelay(15);
  while (1) {
    for (int i = 0; i < 100000; i++) {
    }
    vTaskDelay(1);
  }
}

extern "C" void app_main() {
  ESP_LOGI("app_main", "Starting scheduler from app_main()");
  queue = xQueueCreate(10, sizeof(Message));
  sem = xPipCreate();
  bin_sem = xSemaphoreCreateBinary();
  debugtool_init();
  // xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 7, NULL);
  // xTaskCreate(sender_task, "sender_task", 4096, (void *)100, 5, NULL);
  // xTaskCreate(sender_task, "sender_task2", 4096, (void *)100, 6, NULL);
  xTaskCreate(test_task, "LowP_task", 4096, (void *)1, 5, NULL);
  xTaskCreate(test_task, "HighP_task", 4096, (void *)25, 7, NULL);
  xTaskCreate(inbetween_task, "MidP_task", 4096, NULL, 6, NULL);
  vTaskStartScheduler();
  /* vTaskStartScheduler is blocking - this should never be reached */
  ESP_LOGE("app_main", "insufficient RAM! aborting");
  abort();
}
