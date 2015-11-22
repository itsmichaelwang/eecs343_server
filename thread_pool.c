#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "thread_pool.h"

/**
 *  @struct pool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

#define MAX_THREADS 20
#define STANDBY_SIZE 8

typedef struct {
    void (*function)(void*);
    void* argument;
} pool_task_t;

typedef struct pool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  pool_task_t *queue;
  int thread_count;
  int task_queue_size_limit;
  int queue_head;
  int queue_tail;
} pool_t;

/**
 * @function void *pool_work(void *pool)
 * @brief the worker thread
 * @param pool the pool which own the thread
 */
static void *thread_do_work(void *thread_pool);
static int get_next_task(pool_t* pool);


/*
 * Create a pool, initialize variables, etc
 *
 */
pool_t *pool_create(int thread_count, int queue_size)
{
  if (thread_count > MAX_THREADS) {
    printf("Too many threads - Can't create that many\n");
    exit(-1);
  }

  int i;

  // Initialize all of the elements of the pool_t struct
  pool_t* pool = (pool_t*)malloc(sizeof(pool_t));
  pthread_mutex_init(&(pool->lock), NULL);
  pthread_cond_init(&(pool->notify), NULL);

  // Array of threads
  pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
  for(i = 0; i < thread_count; i++)
    pthread_create(&(pool->threads[i]), NULL, thread_do_work, (void*)pool);

  // Queue is an array of size queue_size + 1 in order to properly queue as an array
  pool->queue = (pool_task_t*)malloc(sizeof(pool_task_t) * (queue_size + 1));
  pool->thread_count = thread_count;
  pool->task_queue_size_limit = queue_size;
  pool->queue_head = 0;
  pool->queue_tail = 0;

  return pool;
}

/*
Function to add a new task to the pool.
Since we are using an array and two indices, if the head is the same as the tail
we know that the queue is empty.
Only one thread calls pool_add_task (the main thread) so we don't have to worry
about threads clashing.
*/

int pool_add_task(pool_t *pool, void (*function)(void*), void* argument)
{
    int err = 0;

    /*
    Our queue is implemented in a circular buffer using an array.
    We ensure that the tail is not equal to the head after something is added so it doesn't
    look like the buffer is empty.
    */

    if ((pool->queue_tail + 1) % (pool->task_queue_size_limit + 1) != pool->queue_head)
    {
      pool->queue[pool->queue_tail].function = function;
      pool->queue[pool->queue_tail].argument = argument;

      // Increment tail index after adding the function so the most recently added task can't
      // be accessed by other threads
      pool->queue_tail = (pool->queue_tail + 1) % (pool->task_queue_size_limit + 1);

      // Notify sleeping threads that a new task has been added to the queue.
      err = pthread_cond_broadcast(&(pool->notify));
      if(err)
        printf("Error: broadcast failed\n");
    }
    else
    {
      // The queue is full.  We cannot add a task. We fail here and return -1
      return -1;
    }
        
    return err;
}



/*
 * Destroy the pool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
    int err = 0;
    int i;

    // Add dummy task to the pool to signal the threads to exit
    // A NULL task is listened for
    while(pool_add_task(pool, NULL, 0));
    

    /* Join all worker thread */
    for(i = 0; i < pool->thread_count; ++i)
    {
      if(pool->threads[i] != pthread_self())
        pthread_join(pool->threads[i], NULL);
    }

    free((void*)pool->queue);
    free((void*)pool->threads);
    free((void*)pool);

    return err;
}

// This function tries to grab a task from the task queue.
// The queue's head lock must be locked when this function is
// called. If a task is found to execute, the lock is released,
// the task is executed, and the function returns 1. If the queue
// is empty, the lock remains locked and the function returns 0.

/*
Function that attempts to get the next task to be executed
*/
static int get_next_task(pool_t* pool)
{
  if(pool->queue_head != pool->queue_tail)
  {
    //queue is not empty
    pool_task_t task = pool->queue[pool->queue_head];

    if(task.function == NULL)
    {
      // This is the dummy task telling the worker threads to exit so we can shut down
      // Release the lock and exit
      pthread_mutex_unlock(&(pool->lock));
      pthread_exit(NULL);
    }

    // Task found, so increment head, release lock and execute that function
    pool->queue_head = (pool->queue_head + 1) % (pool->task_queue_size_limit + 1);
    pthread_mutex_unlock(&(pool->lock));
    (task.function)(task.argument);
    return 1;
  }

  // Queue was empty
  return 0;
}

/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *thread_pool)
{ 
    pool_t* pool = (pool_t*)thread_pool;

    while(1) {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(pool->lock));

        // check whether there is another task to be executed before going to sleep
        if(get_next_task(pool))
          continue;

        // Wait on condition variable.
      
        pthread_cond_wait(&(pool->notify), &(pool->lock));

        // If the queue is empty after we are woken up, then simply release the lock
        if(!get_next_task(pool))
          pthread_mutex_unlock(&(pool->lock));
    }

    pthread_exit(NULL);
    return(NULL);
}