#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

struct pool_t;

struct pool_t* pool_create(int thread_count, int queue_size);
int pool_add_task(struct pool_t* pool, void (*routine)(void *), void *arg);
int pool_destroy(struct pool_t* pool);

#endif
