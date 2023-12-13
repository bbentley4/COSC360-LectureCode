#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <linux/sched.h>
#include <sched.h>
// #define FIXED
#define TRIALS 100000000

typedef struct 
{
    int trials;
    int cnt;
    sem_t mutex;
    sem_t done;
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
    Data *d;
    int shmfd;
    shmfd = shm_open("/data.shm", O_RDWR, 0);
    if (shmfd < 0) {
    perror("shm_open");
    return -1;
    }
    d = mmap(NULL, sizeof(*d), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (d == MAP_FAILED) {
    perror("mmap");
    return -1;
    }
    close(shmfd);
    count(d);
    sem_wait(&d->mutex);
    d->trials += TRIALS;
    sem_post(&d->mutex);
    sem_post(&d->done);
    munmap(d, sizeof(*d));
    shm_unlink("/data.shm");
}
