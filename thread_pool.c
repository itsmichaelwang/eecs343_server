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

#define MAX_THREADS 5
#define STANDBY_SIZE 10


// function pointer to two different functions (parse_request or process request)
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
  int head;
  int tail;
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
    threadpool->queue = (pool_task_t*)malloc(sizeof(pool_task_t) * (queue_size + 1));

    //thread count as the number of threads
    threadpool->thread_count = num_threads;
    threadpool->task_queue_size_limit = queue_size;

    threadpool->head = 0;
    threadpool->tail = 0;
    return threadpool;
}


/*
 * Add a task to the threadpool
 * Task contains both pointers to the process_request and parse_request
 */


int pool_add_task(pool_t *pool, void (*function)(void *), void *argument)
{
    int err = 0;
    int end = pool->tail;

    pthread_mutex_lock(&pool->lock);
    //Append the task's function(parse or process request and the arguments
    pool->queue[end].function = function;
    pool->queue[end].argument = argument;
    printf("task queue size limit%d", pool->task_queue_size_limit);

    // Increment tail index after adding the function so the task added at the end cannot be accessed by other threads
    end = (end + 1) % (pool->task_queue_size_limit + 1);
    printf("End%d", end);

    /* Signal waiting threads. */
    pthread_cond_signal(&pool->notify);
    /* pthread_cond_signal wakes up/notifies another thread that is is waiting on another condition
    variable It should be called after mutex is locked, and must unlock mutex in order for pthread_cond_wait() routine to complete.*/

    pthread_mutex_unlock(&pool->lock);

    return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int pool_destroy(pool_t *pool)
{
    int err = 0;
    int i = 0;

    //join each thread when pool gets destroyed
    for(i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    //destroy mutex and the conditon variable
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);

    free(pool->queue);
    free(pool->threads);
    free(pool);

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
