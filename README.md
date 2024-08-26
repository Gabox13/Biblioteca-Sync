Biblioteca implementada por Gabriel Jiménez Ocampo y Lucas Alexander Oviedo Castro

# Biblioteca Sync escrita en C usando hilos y mutex 

Biblioteca que implementa 3 estructuras de sincronización basadas en el mutex que implementa C utilizando distintas lógicas
dependiendo de un comportamiento esperado de los hilos de C.

# Pre-requisitos
- C

## Tabla de Contenidos
- [Implementación](#implementación)
    - [Semáforo](#semáforo)
    - [Barrera](#barrera)
    - [Read Write Lock](#read-write-lock)
- [Uso de cada estructura](#uso-de-cada-estructura)
    - [Semáforo](#uso-de-semáforo)
    - [Barrera](#uso-de-barrera)
    - [Read Write Lock](#uso-de-read-write-lock)

    
# Implementación

La biblioteca como tal está basada en Mutex o exclusión mutua que está implementada en C. Estas implementaciones aplican lógica según lo necesitado para cada tipo de estructura, ya que 
mutex forma parte de la estructura básica relacionada a la sincronización de hilos. Por ende, utilizar mutex como la base o el candado para crear estructuras que trabajen con cierto comportamiento es lo más adecuado.
Ya que se está trabajando con hilos sincronizados, es necesario llevar un orden de cómo avanzan y realizan sus tareas. Además, es necesario inicializar los hilos y realizar un include de la biblioteca Pthread para crear hilos.
`#include <pthread.h>`

## Semáforo
Para esta implementación se utiliza un `struct` que va a ser nuestro semáforo. Este tiene dos valores: el `mutex` y un `value`, el cual corresponde a un contador de hilos desocupados. Si es menor o igual a cero, significa que no hay hilos desocupados y que tendrán que esperar para adquirir el semáforo.

```c
typedef struct {
  pthread_mutex_t mutex;
  int value;
} semaphore;
```

El semáforo se basa en dos funciones. La función wait(semaphore) bloquea el mutex y, mientras no haya ningún hilo desocupado (que el valor del semáforo sea menor o igual que 0), se mantendrá esperando.
```C
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
```
Cuando alguno se desocupe, se resta el valor al semáforo y se desbloquea el mutex.

La función signal(semaphore) bloquea el mutex y suma un valor al semáforo para indicar que hay un hilo más ocupado. Finalmente, desbloquea el mutex.
```C
void signal(semaphore *sem) {
  pthread_mutex_lock(&sem->mutex);
  sem->value++;
  printf("Thread %ld: liberó el semáforo\n", pthread_self());
  pthread_mutex_unlock(&sem->mutex);
}
```
## Barrera

La implementación de la barrera está basada en una barrera de hilos, esta barrera recibe como parámetros la cantidad de hilos, el mutex para lograr los lock y la estructura de barrera en sí misma para lograr su construcción. 

```C
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int count;
  int num_threads;
} barrier_t;
```
Teniendo la barrera inicializada, se crearon dos métodos más que son los que permiten llevar la lógica y el conteo de los hilos que van llegando a cierta parte del código donde necesitamos que no avancen más.


La función barrier_wait() recibe un puntero a la estructura para lograr utilizar el mutex dentro de la función, como primer paso se bloquea el mutex apenas entra el hilo para que ninguno más entre hasta que se verifique si es el último hilo que se está esperando o hay que pasarlo a una condición de espera y después se abre el mutex para la entrada del próximo hilo.
```C
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
```

En caso de que sea el último hilo que se espera, se hace un broadcast diciéndole a todos los hilos que pueden avanzar y se desbloquea el mutex para seguir con la ejecución.

## Read Write Lock
En el caso de la implementación de Read Write Lock se decidió utilizar una versión en donde solo se utiliza un mutex y una variable condicional para el avance de los hilos. Se creó una estructura que permite contener un contador de lectores, escritores, una bandera si hay un escritor haciendo cambios, una variable para el mutex y otra variable para la condicional.

```C
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int writers;
  int readers;
  int writer_active;
} read_write_lock;
```
Después de crear un “constructor” para la estructura se programó la función begin_read(), esta función permite llevar el conteo de lectores que están entrando para leer el recurso. El primer paso cuando entra el hilo es cerrar el mutex para poder llevar un orden, antes de lograr leer se verifica que no haya escritores esperando o dentro del recurso, en dado caso que si haya escritor se activará la bandera await en el mutex hasta que el escritor salga, después de que sale se aumenta el contador de lectores y se vuelve a desbloquear el mutex.

```C
void begin_read(read_write_lock *rwl) {
  pthread_mutex_lock(&rwl->mutex);
  while (rwl->writer_active || rwl->writers > 0) {
    pthread_cond_wait(&rwl->cond, &rwl->mutex);
  }
  rwl->readers++;
  pthread_mutex_unlock(&rwl->mutex);
}
```


La función end_read() permite avisar al Read Write Lock que un lector ya termino de leer y va a salir del recurso, por ende el contador de lectores disminuye, esta misma función revisa si ya no hay lectores dentro del recurso para permitir al escritor entrar en dado caso que haya uno si no simplemente abre el mutex de nuevo. 

```C
void end_read(read_write_lock *rwl) {
  pthread_mutex_lock(&rwl->mutex);
  rwl->readers--;
  if (rwl->readers == 0) {
    pthread_cond_broadcast(&rwl->cond);
  }
  pthread_mutex_unlock(&rwl->mutex);
}
```

La función begin_write() le da la capacidad de avisar que un hilo necesita realizar una escritura en el recurso por ende aumenta la variable que lleva el conteo de escritores  y verifica que no haya un escritor dentro o hayan lectores, en dado caso que haya lectores el escrito se queda en espera, sino el contador de escritores disminuye y activa la bandera que hay un escritor dentro y lo deja pasar.

```C
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
```

La función end_write() permite avisar al lock de que un escritor terminó por ende desactiva la bandera que avisa que hay un escritor dentro, aplica el broadcast de que ya pueden pasar el próximo escritor o los lectores y desbloquea el mutex.

```C
void end_write(read_write_lock *rwl) {
  pthread_mutex_lock(&rwl->mutex);
  rwl->writer_active = 0;

  // avisar a todos los hilos
  pthread_cond_broadcast(&rwl->cond);

  pthread_mutex_unlock(&rwl->mutex);
}
```

# Uso de cada estructura 

En primer lugar, para lograr usar cada estructura y sus dichas funciones es necesario importar los por medio de su archivo header como usualmente se hace en C.


```C
#include "barrier.h"
#include "rwlock.h"
#include "semaphore.h"
```

## Uso de semáforo

Primero es necesario crear la estructura del semáforo para lograr enviarlo a las funciones donde sea necesario contener el tráfico.

```C
semaphore *sem = malloc(sizeof(semaphore));
pthread_mutex_init(&sem->mutex, NULL);
sem->value = 2;
```

Ahora tomando en cuenta que trabajamos con hilos, es necesario hacer referencia que cada vez que se cree un hilo mandarlo a la función donde ocupemos realizar la acción en especifica en esta función vendrá la lógica de donde necesitemos aplicar el wait
del semáforo.
```C
  wait(sem);
  sleep(2); // Exactamente en esta parte vendría la lógica que se necesita para trabajar entre hilos usando el semáforo como lógica.
  signal(sem);
```

Después de la lógica aplicada se envía el semáforo al signal para lograr liberarlo y permite el paso de otros hilos.

Si quiere ver el ejemplo en ejecucion solo debe llamar `semaphore_test()` del `semaphore.h` en su main de C.

## Uso de barrera

Para el caso de la barrera es necesario inicializarla con el método ` barrier_init(barrier_t *barrier, int num_threads)` 
este metodo recibe un puntero a una estructura tipo barrier_t y el número de hilos esperados para liberar la barrera.

```C
  int num_threads = 2;
  pthread_t threads[2];
  barrier_t barrier;
  barrier_init(&barrier, num_threads);
```

Con la estructura inicializada muy parecido al ejemplo anterior se crean los hilos y al mismo tiempo se envían a una función en donde se maneje toda la lógica y después llamar la función llamada barrier_wait(barrier)
que recibe la barrera para llevar el control de la lógica y espera que la restricción de la cantidad de hilos se cumpla, después de que se cumpla todos los hilos salen de la barrera.

```C
  printf("Thread esperando en la barrera...\n");
  barrier_wait(barrier);
  printf("Thread pasado la barrera!\n");
```

Si quiere ver el ejemplo corriendo llame al metodo `barrier_test();` del `barrier.h` en su main de C.

## Uso de Read Write Lock 

Con read write lock es lo mismo tenemos una función que inicializa la estructura que recibe un puntero del tipo read_write_lock y inicializa sus parámetros.
`read_write_lock_init(&rwlock)`

Teniendo inicializado el rwLock creamos los hilos y los mandamos a las respectivas funciones donde se maneja la lógica de escritura y lectura, evidentemente los begins y los ends van a depender de cuales hilos queremos que lean o escriban en el recurso.

```C
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
```
En el ejemplo anterior mandamos 5 hilos a leer y 2 a escribir, por ende, la función reader tiene el begin read y el end read, y el write tiene begin write y end write
Cada una con su respectiva lógica dependiendo de lo que se necesite realizar.

```C
  int id = *((int *)arg);
  begin_read(&rwlock);
  printf("Reader %d: Leyendo el valor %d\n", id, shared_data);
  usleep(100000); // Simula tiempo de lectura
  end_read(&rwlock);
  return NULL;
```
La lógica debe ir donde se simula el tiempo de lectura en el ejemplo, en el caso de leer.

```C
  int id = *((int *)arg);
  begin_write(&rwlock);
  shared_data++;
  printf("Writer %d: Escribiendo el valor %d\n", id, shared_data);
  usleep(100000); // Simula tiempo de escritura
  end_write(&rwlock);
  return NULL;
```
La lógica debe ir donde se simula el tiempo de escritura en el ejemplo, en el caso de escribir.

Si quiere ver el ejemplo corriendo llame al metodo `read_write_lock_test();` del `rwlock.h` en su main de C.
