#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#define _GNU_SOURCE
#include <linux/sched.h>
#include <sched.h>

int clone(int (*fn)(void*), void *stack, int flags, void *arg, ...);

#define FIXED
#define TRIALS 100000000

typedef struct {
    int cnt;
    sem_t mutex;
} Data;

int goodcnt(void *a)
{
    Data *d = (Data *)a;
    int i, tmp;
    sem_wait(&d->mutex);
    for(i = 0; i < TRIALS; i++)
    {
        tmp = d->cnt;
        tmp += 1;
        d->cnt = tmp;
    }
    sem_post(&d->mutex);
    return 0;
}

int badcnt(void *a)
{
    Data *d = (Data *)a;
    int i, tmp;
    for(i = 0; i < TRIALS; i++)
    {
        tmp = d->cnt;
        tmp += 1;
        d->cnt = tmp;
    }
    return 0;
}

#ifdef FIXED

int (*count)(void *) = goodcnt;
#else
int (*count)(void *) = badcnt;
#endif

int main() 
{
    int child1_pid;
    int child2_pid;
    char *stack1, *stack2;
    Data d;
    d.cnt = 0;
    sem_init(&d.mutex, 0, 1);
    stack1 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS |
    MAP_STACK, -1, 0);
    stack2 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS |
    MAP_STACK, -1, 0);
    child1_pid = clone(count, stack1 + 4096, CLONE_VM | SIGCHLD | CLONE_FS, &d);
    child2_pid = clone(count, stack2 + 4096, CLONE_VM | SIGCHLD | CLONE_FS, &d);
    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);
    if (d.cnt < 2 * TRIALS) 
    {
        printf("BAD: cnt is %d, should be %d\n", d.cnt, 2*TRIALS);
    }
    else 
    {
        printf("GOOD: cnt is %d\n", d.cnt);
    }
    sem_destroy(&d.mutex);
    munmap(stack1, 4096);
    munmap(stack2, 4096);
}
