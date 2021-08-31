#ifndef KNET_THREADPOOL_H
#define KNET_THREADPOOL_H

#include <sys/sysinfo.h>
#include <pthread.h>

#define DEFAULT_THREAD_QUANTITY get_nprocs()

struct job {
    void *(*function)(void *);
    void *arg;
    struct job *next;
};

struct job_queue {
    struct job *head, *tail;
    int size;
};

struct thread_pool {
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int size;
    int jobCounter;
    int stop;
    struct job_queue jobQueue;
};

struct thread_pool thread_pool_create( int size );
void thread_pool_start( struct thread_pool *threadPoolPointer );
void thread_pool_add_job( struct thread_pool *threadPoolPointer, void *(*function)(void *), void *arg );
void thread_pool_destroy( struct thread_pool threadPool );

#endif // KNET_THREADPOOL_H