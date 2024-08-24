#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <pthread.h>

typedef struct {
  pthread_mutex_t mutex;
  int value;
} semaphore;

void wait(semaphore *sem);
void signal(semaphore *sem);
void *semaphore_func(void *arg);
void semaphore_test();

#endif