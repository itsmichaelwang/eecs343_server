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
  int head;
  int tail;
} pool_t;

/**
 * @function void *pool_work(void *pool)
 * @brief the worker thread
 * @param pool the pool which own the thread
 */
static void *thread_do_work(void *thread_pool);

/*
 * Create a pool, initialize variables, etc
 *
 */
pool_t *pool_create(int thread_count, int queue_size)
{

  int i;
  pool_t* pool = (pool_t*)malloc(sizeof(pool_t));
  pthread_mutex_init(&(pool->lock), NULL);
  pthread_cond_init(&(pool->notify), NULL);

  pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
  for(i = 0; i < thread_count; i++)
    pthread_create(&(pool->threads[i]), NULL, thread_do_work, (void*)pool);

  pool->queue = (pool_task_t*)malloc(sizeof(pool_task_t) * (queue_size + 1));
  pool->thread_count = thread_count;
  pool->task_queue_size_limit = queue_size;
  pool->head = 0;
  pool->tail = 0;

  return pool;
}


int pool_add_task(pool_t *pool, void (*function)(void*), void* argument)
{
    int err = 0;
    int end = pool->tail;
    if ((end + 1) % (pool->task_queue_size_limit + 1) != pool->head)
    {
      pool->queue[end].function = function;
      pool->queue[end].argument = argument;

      end = (end + 1) % (pool->task_queue_size_limit + 1);
      err = pthread_cond_signal(&(pool->notify));
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

    
    /* Join all worker thread */
    for(i = 0; i < pool->thread_count; ++i)
    {
        pthread_join(pool->threads[i], NULL);
    }

    free((void*)pool->queue);
    free((void*)pool->threads);
    free((void*)pool);

    return err;
}

static void *thread_do_work(void *thread_pool)
{ 
    pool_t* pool = (pool_t*)thread_pool;

    while(1) {
        pthread_mutex_lock(&(pool->lock));

        if(pool->head != pool->tail)
        {
          pool_task_t task = pool->queue[pool->head];

          if(task.function == NULL)
          {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
          }
          pool->head = (pool->head + 1) % (pool->task_queue_size_limit + 1);
          pthread_mutex_unlock(&(pool->lock));
          (task.function)(task.argument);
          return 1;
        }
      
        pthread_cond_wait(&(pool->notify), &(pool->lock));

        if(pool->head != pool->tail)
        {
          pool_task_t task = pool->queue[pool->head];

          if(task.function == NULL)
          {

            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
          }

          pool->head = (pool->head + 1) % (pool->task_queue_size_limit + 1);
          pthread_mutex_unlock(&(pool->lock));
          (task.function)(task.argument);
          return 0;
        }
    }

    pthread_exit(NULL);
    return(NULL);
}