#ifndef RWLOCK_H
#define RWLOCK_H

#include <pthread.h>

// Definición de la estructura read_write_lock
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int writers;
  int readers;
  int writer_active;
} read_write_lock;

// Declaraciones de las funciones
void read_write_lock_init(read_write_lock *rwl);
void begin_read(read_write_lock *rwl);
void end_read(read_write_lock *rwl);
void begin_write(read_write_lock *rwl);
void end_write(read_write_lock *rwl);
void read_write_lock_test(void);
void *reader(void *arg);
void *writer(void *arg);

#endif // READ_WRITE_LOCK_H