// Demonstrates a conditional variable
#define _GNU_SOURCE // gettid() may not exist for non-Linux OSes
#include <pthread.h>
#include <unistd.h> // getpid(), gettid()
#include <stdio.h>
#include <time.h> // nanosleep()
#define NUM_THREADS 5

typedef struct {
pthread_mutex_t lock;
pthread_cond_t condvar;
} Data;

void *worker(void *);

int main(int argc, char *argv[])
{
    Data data = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .condvar = PTHREAD_COND_INITIALIZER,
    };
    
    pthread_t tids[NUM_THREADS];
    int i;
    time_t sleep_time;
    if (argc < 2) 
    {
        printf("Usage: %s <seconds before broadcast>.\n", argv[0]);
        return -1;
    }
    if (sscanf(argv[1], "%lu", &sleep_time) != 1) 
    {
        printf("Invalid sleep time specified: '%s'\n", argv[1]);
        return -1;
    }
    for (i = 0;i < NUM_THREADS;i+=1) 
    {
        pthread_create(tids + i, NULL, worker, &data);
    }

    // Only those waiting on the conditional signal can actually hear it.
    // If a child gets to the signal too late, then the child won't be signalled
    // and will be stuck waiting.
    if (argc >= 2) 
    {
        nanosleep(&(struct timespec){sleep_time,0},NULL);
    }
    pthread_cond_broadcast(&data.condvar);

    for (i = 0;i < NUM_THREADS;i+=1) 
    {
        pthread_join(tids[i], NULL);
    }
}

void *worker(void *d)
{
    Data *data = (Data *)d;
    printf("Worker (pid: %u, tid: %u): waiting on condvar.\n", getpid(), gettid());
    pthread_mutex_lock(&data->lock);
    pthread_cond_wait(&data->condvar, &data->lock);
    printf("Worker (pid: %u, tid: %u): got signaled!\n", getpid(), gettid());
    pthread_mutex_unlock(&data->lock);
    return NULL;
}
