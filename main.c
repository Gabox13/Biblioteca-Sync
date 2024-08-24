#include "barrier.h"
#include "rwlock.h"
#include "semaphore.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  read_write_lock_test();
  semaphore_test();
  barrier_test();

  return 0;
}