#include "rwlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

read_write_lock rwlock;
int shared_data = 0;

void read_write_lock_init(read_write_lock *rwl) {
  rwl->writers = 0;
  rwl->readers = 0;
  rwl->writer_active = 0;
  pthread_mutex_init(&rwl->mutex, NULL);
  pthread_cond_init(&rwl->cond, NULL);
}

void begin_read(read_write_lock *rwl) {
  pthread_mutex_lock(&rwl->mutex);
  while (rwl->writer_active || rwl->writers > 0) {
    pthread_cond_wait(&rwl->cond, &rwl->mutex);
  }
  rwl->readers++;
  pthread_mutex_unlock(&rwl->mutex);
}

void end_read(read_write_lock *rwl) {
  pthread_mutex_lock(&rwl->mutex);
  rwl->readers--;
  if (rwl->readers == 0) {
    pthread_cond_broadcast(&rwl->cond);
  }
  pthread_mutex_unlock(&rwl->mutex);
}

void begin_write(read_write_lock *rwl) {
  pthread_mutex_lock(&rwl->mutex);
  rwl->writers++;
  while (rwl->readers > 0 || rwl->writer_active) {
    pthread_cond_wait(&rwl->cond, &rwl->mutex);
  }
  rwl->writers--;
  rwl->writer_active = 1;
  pthread_mutex_unlock(&rwl->mutex);
}

void end_write(read_write_lock *rwl) {
  pthread_mutex_lock(&rwl->mutex);
  rwl->writer_active = 0;

  // avisar a todos los hilos
  pthread_cond_broadcast(&rwl->cond);

  pthread_mutex_unlock(&rwl->mutex);
}

void read_write_lock_test(void) {
  pthread_t readers[5], writers[2];
  int reader_ids[5], writer_ids[2];

  read_write_lock_init(&rwlock);

  // Crear hilos lectores
  for (int i = 0; i < 5; i++) {
    reader_ids[i] = i + 1;
    pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
  }

  // Crear hilos escritores
  for (int i = 0; i < 2; i++) {
    writer_ids[i] = i + 1;
    pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
  }

  // Esperar a que todos los hilos terminen
  for (int i = 0; i < 5; i++) {
    pthread_join(readers[i], NULL);
  }

  for (int i = 0; i < 2; i++) {
    pthread_join(writers[i], NULL);
  }
}

void *reader(void *arg) {
  int id = *((int *)arg);
  begin_read(&rwlock);
  printf("Reader %d: Leyendo el valor %d\n", id, shared_data);
  usleep(100000); // Simula tiempo de lectura
  end_read(&rwlock);
  return NULL;
}

void *writer(void *arg) {
  int id = *((int *)arg);
  begin_write(&rwlock);
  shared_data++;
  printf("Writer %d: Escribiendo el valor %d\n", id, shared_data);
  usleep(100000); // Simula tiempo de escritura
  end_write(&rwlock);
  return NULL;
}