#pragma once
#include "freertos/ringbuf.h"

typedef struct {
  size_t num_tasks;
  TickType_t ticksToRun;
} MetaTask;

typedef struct {
  QUEUE_EVENT event;
  TickType_t tick;
  uint32_t timestamp;
  TaskHandle_t taskhandle;
  void *generic_data;
} LogMessage;

void print_logmessage(LogMessage *lm);
void debugtool_task(void *pvParameters);
void debugtool_init();
