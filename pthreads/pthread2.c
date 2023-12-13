#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
// Locking and unlocking a mutex is slow. The define below
// if uncommented will lock and unlock the mutex per iteration
// of the TIRALS for loop.
// If the following is commented-out, then the mutex will be
// locked and unlocked ONCE per thread.
// This demonstrates that you need to be careful about when
// a mutex should be locked.
// #define SLOW
static void *thread_func(void *arg);
#define TRIALS 10000000L
// We can statically create a mutex by setting it equal
// to the constant PTHREAD_MUTEX_INITIALIZER.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void)
{
    pthread_t tids[2];
    long data = 0;
    srand(time(NULL));
    pthread_create(&tids[0], NULL, thread_func, &data);
    pthread_create(&tids[1], NULL, thread_func, &data);
    // This may be 8 and 9 or it might be 9 and 9
    // This is called a *data race* since the output
    // depends on when the second thread gets started.
    pthread_join(tids[0], NULL);
    pthread_join(tids[1], NULL);
    printf("data = %ld\n", data);

    if (data != TRIALS * 2) 
    {
        printf(" -> FAILED (should be %ld).\n", TRIALS * 2);
    }
    else 
    {
        printf(" -> CORRECT.\n");
    }
}

static void *thread_func(void *arg)
{
    long *data = (long *)arg;
    #ifdef SLOW
    for (int i = 0;i < TRIALS;i++) {
    pthread_mutex_lock(&mutex);
    *data += 1;
    pthread_mutex_unlock(&mutex);
    }
    #else
    pthread_mutex_lock(&mutex);
    for (int i = 0;i < TRIALS;i++) 
    {
        *data += 1;
    }
    pthread_mutex_unlock(&mutex);
    #endif
    return NULL;
}
