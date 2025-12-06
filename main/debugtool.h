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

typedef struct {
  TickType_t tick;
  TickType_t new_tick;
  uint32_t timestamp;
} IncrementTickMessage;

typedef struct {
  TASK_EVENT event;
  TickType_t tick;
  uint32_t timestamp;
  void *taskidentifier;
} TaskMessage;

void print_logmessage(LogMessage *lm);
void print_incrementTickMessage(IncrementTickMessage *im);
void print_taskMessage(TaskMessage *_tm);
void debugtool_task(void *pvParameters);
void debugtool_init();
