#pragma once
#include "esp_log.h"

typedef enum {
  QUEUE_EVENT_RECEIVE,
  QUEUE_EVENT_RECEIVE_FAILED,
  QUEUE_EVENT_RECEIVE_FROM_ISR,
  QUEUE_EVENT_RECEIVE_FROM_ISR_FAILED,
  QUEUE_EVENT_SEND,
  QUEUE_EVENT_SEND_FAILED,
  QUEUE_EVENT_SEND_FROM_ISR,
  QUEUE_EVENT_SEND_FROM_ISR_FAILED
} QUEUE_EVENT;
typedef enum {
  TASK_EVENT_CREATE,
  TASK_EVENT_CREATE_FAILED,
  TASK_EVENT_DELETE,
  TASK_EVENT_DELAY,
  TASK_EVENT_DELAY_UNTIL,
  TASK_EVENT_SWITCHED_IN,
  TASK_EVENT_SWITCHED_OUT
} TASK_EVENT;
 
void tracequeue_function(QUEUE_EVENT e, void *pxQueue);
void tracetick_function(uint32_t xTickCount);
void tracetask_function(TASK_EVENT t, void *xTask);
void tracetaskdelay_function(TASK_EVENT t, uint32_t xTicks, int isAbsolute);

#define DEBUG_TAG DRAM_STR("DEBUG")
#define traceQUEUE_RECEIVE(pxQueue)                                            \
  tracequeue_function(QUEUE_EVENT_RECEIVE, (void *)pxQueue);
#define traceQUEUE_RECEIVE_FAILED(pxQueue)                                     \
  tracequeue_function(QUEUE_EVENT_RECEIVE_FAILED, (void *)pxQueue);
#define traceQUEUE_RECEIVE_FROM_ISR(pxQueue)                                   \
  tracequeue_function(QUEUE_EVENT_RECEIVE_FROM_ISR, (void *)pxQueue);
#define traceQUEUE_RECEIVE_FROM_ISR_FAILED(pxQueue)                            \
  tracequeue_function(QUEUE_EVENT_RECEIVE_FROM_ISR_FAILED, (void *)pxQueue);
#define traceQUEUE_SEND(pxQueue)                                               \
  tracequeue_function(QUEUE_EVENT_SEND, (void *)pxQueue);
#define traceQUEUE_SEND_FAILED(pxQueue)                                        \
  tracequeue_function(QUEUE_EVENT_SEND_FAILED, (void *)pxQueue);
#define traceQUEUE_SEND_FROM_ISR(pxQueue)                                      \
  tracequeue_function(QUEUE_EVENT_SEND_FROM_ISR, (void *)pxQueue);
#define traceQUEUE_SEND_FROM_ISR_FAILED(pxQueue)                               \
  tracequeue_function(QUEUE_EVENT_SEND_FROM_ISR_FAILED, (void *)pxQueue);
#define traceTASK_INCREMENT_TICK(xTickCount)                                   \
  tracetick_function(xTickCount);
#define traceTASK_CREATE(xTask)                                                \
  tracetask_function(TASK_EVENT_CREATE, (void *)xTask);
#define traceTASK_CREATE_FAILED()                                              \
  tracetask_function(TASK_EVENT_CREATE_FAILED, NULL);
#define traceTASK_DELETE(xTask)                                                \
  tracetask_function(TASK_EVENT_DELETE, (void *)xTask);
#define traceTASK_DELAY()                                                      \
  tracetaskdelay_function(TASK_EVENT_DELAY, xTicksToDelay, 0)
#define traceTASK_DELAY_UNTIL(xTimeToWake)                                     \
  tracetaskdelay_function(TASK_EVENT_DELAY_UNTIL, xTimeToWake, 1);
#define traceTASK_SWITCHED_IN()                                                \
  tracetask_function(TASK_EVENT_SWITCHED_IN, NULL);
#define traceTASK_SWITCHED_OUT()                                               \
  tracetask_function(TASK_EVENT_SWITCHED_OUT, NULL);
