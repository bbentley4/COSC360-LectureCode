// Demonstrates a conditional variable
#define _GNU_SOURCE // gettid() may not exist for non-Linux OSes
#include <pthread.h>
#include <unistd.h> // getpid(), gettid()
#include <stdio.h>
#include <time.h>
#define NUM_THREADS 5

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t condvar;
} Data;

void *worker(void *);

int main(void)
{
    Data data = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .condvar = PTHREAD_COND_INITIALIZER,
    };

    pthread_t tids[NUM_THREADS];
    int i;
    int num_signals = 0;

    for (i = 0;i < NUM_THREADS;i+=1) 
    {
        pthread_create(tids + i, NULL, worker, &data);
    }

    for (;;) 
    {
        nanosleep(&(struct timespec){1,0}, NULL);
        printf("Press ENTER to signal ('q' to quit): ");
        if (getchar() == 'q') 
        {
            if (num_signals < NUM_THREADS) 
            {
                printf("You only signalled %d times for %d threads. Expect this to hang!\n", num_signals, NUM_THREADS);
            }
            break;
        }
        num_signals += 1;
        pthread_cond_signal(&data.condvar);
    }

    for (i = 0;i < NUM_THREADS;i+=1) 
    {
        pthread_join(tids[i], NULL);
    }
}

void *worker(void *d)
{
    Data *data = (Data *)d;
    printf("Worker (pid: %u, tid: %u, pthread_id: %lu): waiting on condvar.\n",
    getpid(), gettid(), pthread_self());
    pthread_mutex_lock(&data->lock);
    pthread_cond_wait(&data->condvar, &data->lock);
    printf("Worker (pid: %u, tid: %u, pthread_id: %lu): got signaled!\n", getpid(),
    gettid(), pthread_self());
    pthread_mutex_unlock(&data->lock);
    return NULL;
}
