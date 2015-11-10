#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "thread_pool.h"

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

#define MAX_THREADS 1
#define STANDBY_SIZE 10


// function pointer to parse request/
typedef struct {
    void (*function)(void *);
    void *argument;
} pool_task_t;


struct pool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  pool_task_t *queue;
  int thread_count;
  int task_queue_size_limit;
  int queue_start;
  int queue_end;
};

static void *thread_do_work(void *pool);

/*
 * Create a threadpool, initialize variables, etc
 *
 */
pool_t *pool_create(int queue_size, int num_threads)
{

    // Initialize all of the elements of the pool_t struct
    pool_t* threadpool = (pool_t*) malloc(sizeof(pool_t));
    pthread_mutex_init(&(threadpool->lock), NULL);
    pthread_cond_init(&(threadpool->notify), NULL);
    threadpool->threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
    // int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    //void *(*start_routine) (void *), void *arg);
    int i;
    for(i = 0; i < num_threads; i++) {
        pthread_create(&(threadpool->threads[i]), NULL, thread_do_work, (void*) threadpool);
    }
    //threadpool queue of many pool tasks
    threadpool->queue = (pool_task_t*)malloc(sizeof(pool_task_t) * (queue_size));

    //thread count as the number of threads
    threadpool->thread_count = num_threads;
    threadpool->task_queue_size_limit = queue_size;

    threadpool->queue_start = 0;
    threadpool->queue_end = 0;
    return threadpool;
}


/*
 * Add a task to the threadpool
 *
 */
int pool_add_task(pool_t *pool, void (*function)(void *), void *argument)
{
    int err = 0;

    return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
    int err = 0;

    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *pool)
{

    while(1) {

    }

    pthread_exit(NULL);
    return(NULL);
}
