#pragma once
#include "freertos/ringbuf.h"

typedef struct {
  size_t num_tasks;
  TickType_t ticksToRun;
} MetaTask;

typedef enum {
  MESSAGE_TYPE_QUEUE,
  MESSAGE_TYPE_TICK,
  MESSAGE_TYPE_TASK,
  MESSAGE_TYPE_TASK_DELAY
} MESSAGE_TYPE;

typedef struct LogMessage {
  MESSAGE_TYPE type = MESSAGE_TYPE_QUEUE;
  QUEUE_EVENT event;
  TickType_t tick;
  uint32_t timestamp;
  TaskHandle_t taskhandle;
  void *generic_data;
  char taskname[configMAX_TASK_NAME_LEN + 1];
} LogMessage;

typedef struct IncrementTickMessage {
  MESSAGE_TYPE type = MESSAGE_TYPE_TICK;
  TickType_t tick;
  TickType_t new_tick;
  uint32_t timestamp;
} IncrementTickMessage;

typedef struct TaskMessage {
  MESSAGE_TYPE type = MESSAGE_TYPE_TASK;
  TASK_EVENT event;
  TickType_t tick;
  uint32_t timestamp;
  void *taskidentifier;
} TaskMessage;

typedef struct TaskDelayMessage {
  MESSAGE_TYPE type = MESSAGE_TYPE_TASK_DELAY;
  TASK_EVENT event;
  TickType_t tick;
  TickType_t tickstodelay;
  uint32_t timestamp;
  void *taskidentifier;
} TaskDelayMessage;

void print_logmessage(LogMessage *lm);
void print_incrementTickMessage(IncrementTickMessage *im);
void print_taskMessage(TaskMessage *_tm);
void print_taskDelayMessage(TaskDelayMessage *tdm);
void debugtool_task(void *pvParameters);
void debugtool_init();
