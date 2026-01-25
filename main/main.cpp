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

static portMUX_TYPE pip_mux = portMUX_INITIALIZER_UNLOCKED;
typedef struct {
  TaskHandle_t owner;
  QueueHandle_t handle;
  UBaseType_t initial_priority;
} PipSemaphore_t;

BaseType_t xPipTake(PipSemaphore_t *semaphore, TickType_t xTicksToWait) {
  TaskHandle_t curr = xTaskGetCurrentTaskHandle();

    // Detect if semaphore is already taken + apply PI BEFORE blocking
    bool entered = false;
    taskENTER_CRITICAL(&pip_mux);
    if (uxSemaphoreGetCount(semaphore->handle) == 0 && semaphore->owner != NULL) {
        entered = true;
        UBaseType_t curr_prio = uxTaskPriorityGet(curr);
        UBaseType_t owner_prio = uxTaskPriorityGet(semaphore->owner);

        if (curr_prio > owner_prio) {
            // temporarily boost owner priority
            vTaskPrioritySet(semaphore->owner, curr_prio + 1);
        }
    }
    taskEXIT_CRITICAL(&pip_mux);
    ESP_LOGI("SEM", "Task %s entered critical section: %d", pcTaskGetName(curr), entered);
    // Block on the semaphore
    if (xSemaphoreTake(semaphore->handle, xTicksToWait) != pdTRUE) {
        return pdFALSE;  // timeout or failure
    }

    // Establish ownership after successfully taking semaphore
    taskENTER_CRITICAL(&pip_mux);
    semaphore->owner = curr;
    semaphore->initial_priority = uxTaskPriorityGet(curr);
    taskEXIT_CRITICAL(&pip_mux);
    ESP_LOGI("SEM", "Task %s now owns semaphore", pcTaskGetName(curr));
    return pdTRUE;
}

BaseType_t xPipGive(PipSemaphore_t *semaphore) {
  taskENTER_CRITICAL(&pip_mux);
    if (semaphore->owner == xTaskGetCurrentTaskHandle()) {
        vTaskPrioritySet(semaphore->owner, semaphore->initial_priority);
        semaphore->owner = NULL;
    }
  taskEXIT_CRITICAL(&pip_mux);

    return xSemaphoreGive(semaphore->handle);
}

PipSemaphore_t xPipCreate() {
  PipSemaphore_t semaphore = {.owner = NULL,
                              .handle = xSemaphoreCreateBinary()};
  xSemaphoreGive(semaphore.handle);
  return semaphore;
}

QueueHandle_t queue;

void sender_task(void *pvParameters) {
  int period = (uint32_t)pvParameters;
  TickType_t t = xTaskGetTickCount();
  Message message;
  sprintf(message.message, "Sender with %d period", period);
  while (1) {
    int32_t now = esp_timer_get_time();
    while (esp_timer_get_time() < now + (period / 4) * 1000)
      ;
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

PipSemaphore_t sem;
SemaphoreHandle_t bin_sem;

// Test Task to see if taskDelay shows on Debugger
void test_task(void *pvParameters) {
  int delay = (int)pvParameters;
  vTaskDelay(delay);
  while (1) {
    ESP_LOGI("SEM", "Attempting to take by: %s at %lu", pcTaskGetName(sem.owner), (uint32_t)esp_timer_get_time());
    xPipTake(&sem, portMAX_DELAY);
    //xSemaphoreTake(bin_sem, portMAX_DELAY);
    ESP_LOGI("SEM", "Took by: %s at %lu", pcTaskGetName(sem.owner), (uint32_t)esp_timer_get_time());
    for (int i = 0; i < 20000; i++) {
    }
    ESP_LOGI("SEM", "Giving back by: %s at %lu", pcTaskGetName(sem.owner), (uint32_t)esp_timer_get_time());
    //vTaskDelay(5);
    xPipGive(&sem);
    //xSemaphoreGive(bin_sem);
    vTaskDelay(5);
  }
}

void inbetween_task(void *pvParameters) {
  int delay = (int)pvParameters;
  vTaskDelay(delay);
  while (1) {
    for (int i = 0; i < 20000; i++) {
    }
    vTaskDelay(1);
  }
}

extern "C" void app_main() {
  ESP_LOGI("app_main", "Starting scheduler from app_main()");
  queue = xQueueCreate(10, sizeof(Message));
  sem = xPipCreate();
  //bin_sem = xSemaphoreCreateBinary();
  //xSemaphoreGive(bin_sem);
  debugtool_init();
  // xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 7, NULL);
  // xTaskCreate(sender_task, "sender_task", 4096, (void *)100, 5, NULL);
  // xTaskCreate(sender_task, "sender_task2", 4096, (void *)100, 6, NULL);
  xTaskCreate(test_task, "LowP_task", 4096, (void *)1, 5, NULL);
  xTaskCreate(test_task, "HighP_task", 4096, (void *)10, 7, NULL);
  xTaskCreate(inbetween_task, "MidP_task", 4096, (void*)4, 6, NULL);
  vTaskStartScheduler();
  /* vTaskStartScheduler is blocking - this should never be reached */
  ESP_LOGE("app_main", "insufficient RAM! aborting");
  abort();
}
