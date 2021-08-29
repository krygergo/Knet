#include <stdlib.h>
#include <stdio.h>

#include "../header/threadpool.h"

static struct job *job_queue_pop_first( struct job_queue *jobQueuePointer ) {
    if( jobQueuePointer->size == 0 ) {
        return NULL;
    } else {
        struct job *job = jobQueuePointer->head;
        jobQueuePointer->head = jobQueuePointer->head->next;
        jobQueuePointer->size--;
        return job;
    }
}

static void *thread_start( void *arg ) {
    struct thread_pool *threadPoolPointer = (struct thread_pool *) arg;
    while( !threadPoolPointer->stop ) {
        pthread_mutex_lock( &threadPoolPointer->mutex );
        while( threadPoolPointer->jobQueue.size == 0 && !threadPoolPointer->stop )
            pthread_cond_wait( &threadPoolPointer->cond, &threadPoolPointer->mutex );
        
        if( threadPoolPointer->jobQueue.size > 0 && !threadPoolPointer->stop ) {
            struct job *job = job_queue_pop_first( &threadPoolPointer->jobQueue );
            pthread_mutex_unlock( &threadPoolPointer->mutex );        
            job->function( job->arg );
            free( job );
        } else {
            pthread_mutex_unlock( &threadPoolPointer->mutex );
        }
    }
    pthread_cond_broadcast( &threadPoolPointer->cond );
}

static struct job_queue job_queue_create() {
    return (struct job_queue) {
        .head = NULL,
        .tail = NULL,
        .size = 0
    };
}

struct thread_pool thread_pool_create( int size ) {
    return (struct thread_pool) {
        .threads = malloc( sizeof( pthread_t ) * size ),
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .size = size,
        .jobCounter = 0,
        .stop = 0,
        .jobQueue = job_queue_create()
    };
}

int thread_pool_start( struct thread_pool *threadPool ) {
    for( int i = 0; i < threadPool->size; i++ ) {
        pthread_create( &threadPool->threads[i], NULL, thread_start, threadPool );
    }
    return 0;
}

static void job_queue_push_last( struct job_queue *jobQueuePointer, struct job *jobPointer, pthread_mutex_t *mutex ) {
    pthread_mutex_lock( mutex );
    if( jobQueuePointer->size == 0 ) {
        jobQueuePointer->head = jobPointer;
        jobQueuePointer->tail = jobPointer;
    } else {
        jobQueuePointer->tail->next = jobPointer;
        jobQueuePointer->tail = jobPointer;
    }
    jobQueuePointer->size++;
    pthread_mutex_unlock( mutex );
}

void thread_pool_add_job( struct thread_pool *threadPoolPointer, void *(*function)(void *), void *arg ) {
    struct job *jobPointer = malloc( sizeof( struct job ) );
    jobPointer->function = function;
    jobPointer->arg = arg;
    jobPointer->next = NULL;
    job_queue_push_last( &threadPoolPointer->jobQueue, jobPointer, &threadPoolPointer->mutex );
    pthread_cond_signal( &threadPoolPointer->cond );
}

int thread_pool_destroy( struct thread_pool threadPool ) {
    for( int i = 0; i < threadPool.size; i++ ) {
        pthread_join( threadPool.threads[i], NULL );
    }
    pthread_mutex_destroy( &threadPool.mutex );
    pthread_cond_destroy( &threadPool.cond );
    free( threadPool.threads );
    return 0;
}
