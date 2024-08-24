#ifndef BARRIER_H
#define BARRIER_H

#include <pthread.h>

// Definición de la estructura barrier_t
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int count;
  int num_threads;
} barrier_t;

// Declaraciones de las funciones
void barrier_init(barrier_t *barrier, int num_threads);
void barrier_destroy(barrier_t *barrier);
void barrier_wait(barrier_t *barrier);
void *barrier_func(void *arg);
void barrier_test(void);

#endif // BARRIER_H