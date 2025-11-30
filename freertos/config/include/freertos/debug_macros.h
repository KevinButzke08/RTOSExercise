#pragma once
#include "esp_log.h"

typedef enum { QUEUE_EVENT_RECEIVE, QUEUE_EVENT_RECEIVE_FAILED, QUEUE_EVENT_RECEIVE_FROM_ISR, QUEUE_EVENT_RECEIVE_FROM_ISR_FAILED,
   QUEUE_EVENT_SEND, QUEUE_EVENT_SEND_FAILED, QUEUE_EVENT_SEND_FROM_ISR, QUEUE_EVENT_SEND_FROM_ISR_FAILED } QUEUE_EVENT;

void tracequeue_function(QUEUE_EVENT e, void *pxQueue);

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
