#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
static void *thread_func(void *arg);
#define TRIALS 1000000L

int main(void)
{
    pthread_t tids[2];
    long data = 0;
    srand(time(NULL));
    pthread_create(&tids[0], NULL, thread_func, &data);
    pthread_create(&tids[1], NULL, thread_func, &data);
    // Join will not return until the thread has finished.
    pthread_join(tids[0], NULL);
    pthread_join(tids[1], NULL);
    // We can't get here until BOTH threads finish their work.
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
    // Both threads are running the following simultaneously.
    for (int i = 0;i < TRIALS;i++) 
    {
        // *data += 1 is actually THREE instructions:
        // (1) ld a0, 0(data)
        // (2) addi a0, a0, 1
        // (3) sd a0, 0(data)
        // Each thread can be *preempted* between any of these
        // instructions, making it store the OLD value into 0(data)
        // instead of the newly updated value.
        *data += 1;
        // See pthread2.c for the fix (using a mutual exclusion (MUTEX) variable).
    }
    return NULL;
}
