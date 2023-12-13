// Demonstrates readers-writers problem
// Compile with flag -DCORRECT to get the correct solution
// CORRECT controls the writer's lock
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#define _GNU_SOURCE
#include <linux/sched.h>
#include <sched.h>

int clone(int (*fn)(void *), void *stack, int flags, void *arg, ...);

// #define CORRECT
#define RDTHREADS 3
#define WRTHREADS 2
#define STACK_SIZE 4096
#define WRTRIALS 121222
#define RDTRIALS 102222
typedef struct {
    sem_t rdlock; // Reader's lock
    sem_t wrlock; // Writer's lock
    sem_t printflock; // printf lock
    int rdcount; // Number of readers
    int wrcount; // Number of times a writer wrote.
    int value; // Value we want to read/write
} Data;

typedef struct {
    Data *d;
    char *stack;
    int pid;
} Thread;

int wr_thread(void *arg)
{
    int i;
    Thread *t = (Thread *)arg;
    for (i = 0;i < WRTRIALS;i+=1) 
    {
        #ifdef CORRECT
        sem_wait(&t->d->wrlock);
        #endif
        t->d->value += 1;
        t->d->wrcount += 1;
        #ifdef CORRECT
        sem_post(&t->d->wrlock);
        #endif
    }
    return 0;
}

int rd_thread(void *arg)
{
    int i;
    Thread *t = (Thread *)arg;
    for (i = 0;i < RDTRIALS;i+=1) 
    {
        sem_wait(&t->d->rdlock);
        // We now have exclusive access to rdcount
        t->d->rdcount += 1;
        if (t->d->rdcount == 1) 
        {
            // We are the first reader, exclude writers.
            sem_wait(&t->d->wrlock);
        }

        // We no longer need rdcount
        sem_post(&t->d->rdlock);
        if (t->d->value != t->d->wrcount) 
        {
            sem_wait(&t->d->printflock);
            printf("Reader %6d: found fault with value %d != %d\n", t->pid, t->d-
            >value, t->d->wrcount);
            sem_post(&t->d->printflock);
        }
        // Now that we're done reading, we need access to
        // rdlock.
        sem_wait(&t->d->rdlock);
        t->d->rdcount -= 1;
        if (t->d->rdcount == 0) {
        // We were the last reader, allow writers.
        sem_post(&t->d->wrlock);
        }
        sem_post(&t->d->rdlock);
    }
    return 0;
}

int main(void)
{
    Data data;
    Thread rdthreads[RDTHREADS];
    Thread wrthreads[WRTHREADS];
    int i;
    sem_init(&data.rdlock, 0, 1);
    sem_init(&data.wrlock, 0, 1);
    sem_init(&data.printflock, 0, 1);
    data.value = 0;

    for (i = 0;i < WRTHREADS;i+=1) 
    {
        wrthreads[i].d = &data;
        wrthreads[i].stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS | MAP_STACK, -1, 0);
        wrthreads[i].pid = clone(wr_thread, wrthreads[i].stack + STACK_SIZE,
        CLONE_VM | SIGCHLD | CLONE_FS, wrthreads + i);
    }
    for (i = 0;i < RDTHREADS;i+=1) 
    {
        rdthreads[i].d = &data;
        rdthreads[i].stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS | MAP_STACK, -1, 0);
        rdthreads[i].pid = clone(rd_thread, rdthreads[i].stack + STACK_SIZE,
        CLONE_VM | SIGCHLD | CLONE_FS, rdthreads + i);
    }
    for (i = 0;i < WRTHREADS;i+=1) 
    {
        waitpid(wrthreads[i].pid, NULL, 0);
        munmap(wrthreads[i].stack, STACK_SIZE);
    }
    for (i = 0;i < RDTHREADS;i+=1) 
    {
        waitpid(rdthreads[i].pid, NULL, 0);
        munmap(rdthreads[i].stack, STACK_SIZE);
    }
    sem_destroy(&data.rdlock);
    sem_destroy(&data.wrlock);
    sem_destroy(&data.printflock);
    if (data.value != data.wrcount) 
    {
    printf("ERROR: Values are not correct.\n");
    }
    else {
    printf("CONGRATS: Values are correct.\n");
    }
    return 0;
}
