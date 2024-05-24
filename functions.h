#pragma once
#define BUFFER_SIZE 10000
#include "Qimplementation.h"

queueptr myqueue;
void error(const char *msg);
void *controller_thread(void *arg);
void *worker_thread(void *arg);