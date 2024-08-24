#include "barrier.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void barrier_init(barrier_t *barrier, int num_threads) {
  barrier->count = 0;
  barrier->num_threads = num_threads;
  pthread_mutex_init(&barrier->mutex, NULL);
  pthread_cond_init(&barrier->cond, NULL);
}

void barrier_destroy(barrier_t *barrier) {
  pthread_mutex_destroy(&barrier->mutex);
  pthread_cond_destroy(&barrier->cond);
}

void barrier_wait(barrier_t *barrier) {
  pthread_mutex_lock(&barrier->mutex);

  barrier->count++;

  if (barrier->count == barrier->num_threads) {
    barrier->count = 0;
    pthread_cond_broadcast(&barrier->cond);
  } else {
    pthread_cond_wait(&barrier->cond, &barrier->mutex);
  }

  pthread_mutex_unlock(&barrier->mutex);
}

void *barrier_func(void *arg) {
  barrier_t *barrier = (barrier_t *)arg;

  printf("Thread esperando en la barrera...\n");
  barrier_wait(barrier);
  printf("Thread pasado la barrera!\n");

  return NULL;
}

void barrier_test() {
  int num_threads = 2;
  pthread_t threads[2];
  barrier_t barrier;

  // Inicializar la barrera
  barrier_init(&barrier, num_threads);

  // Crear los threads
  for (int i = 0; i < num_threads; i++) {
    pthread_create(&threads[i], NULL, (void *)barrier_func, (void *)&barrier);
  }

  // Esperar a que todos los threads terminen
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
}