#include "semaphore.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void wait(semaphore *sem) {
  pthread_mutex_lock(&sem->mutex);
  while (sem->value <= 0) {
    pthread_mutex_unlock(&sem->mutex);
    printf("Thread %ld: esperando\n", pthread_self());
    sleep(1);
    pthread_mutex_lock(&sem->mutex);
  }
  sem->value--;
  printf("Thread %ld: adquirió el semáforo\n", pthread_self());
  pthread_mutex_unlock(&sem->mutex);
}

void signal(semaphore *sem) {
  pthread_mutex_lock(&sem->mutex);
  sem->value++;
  printf("Thread %ld: liberó el semáforo\n", pthread_self());
  pthread_mutex_unlock(&sem->mutex);
}

void *semaphore_func(void *arg) {
  semaphore *sem = (semaphore *)arg;
  wait(sem);
  sleep(2);
  signal(sem);
  return NULL;
}

void semaphore_test() {
  pthread_t threads[5];
  semaphore *sem = malloc(sizeof(semaphore));
  pthread_mutex_init(&sem->mutex, NULL);
  sem->value = 2;

  for (int i = 0; i < 5; i++) {
    pthread_create(&threads[i], NULL, semaphore_func, sem);
    sleep(1);
  }
  for (int i = 0; i < 5; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&sem->mutex);
  free(sem);
}