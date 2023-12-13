#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define _GNU_SOURCE
#include <linux/sched.h>
#include <sched.h>

typedef struct {
    int trials;
    int cnt;
    sem_t mutex;
    sem_t done;
} Data;

int main() 
{
    Data *d;
    int v = 0;
    int shmfd;
    shmfd = shm_open("/data.shm", O_CREAT | O_EXCL | O_RDWR, 0666);
    if (shmfd < 0) 
    {
        perror("shm_open");
        return -1;
    }
    d = mmap(NULL, sizeof(*d), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    
    if (d == MAP_FAILED) 
    {
        perror("mmap");
        return -1;
    }

    ftruncate(shmfd, sizeof(*d));
    memset(d, 0, sizeof(*d));
    close(shmfd);

    if (sem_init(&d->mutex, 1, 1) < 0) 
    {
        perror("sem_init");
        return -1;
    }
    if (sem_init(&d->done, 1, 0) < 0) 
    {
        perror("sem_init");
        return -1;
    }

    while (!sem_getvalue(&d->done, &v) && v != 2);

    if (d->cnt < d->trials) 
    {
        printf("BAD: cnt is %d, should be %d\n", d->cnt, d->trials);
    }
    else 
    {
        printf("GOOD: cnt is %d\n", d->cnt);
    }
    sem_destroy(&d->mutex);
    sem_destroy(&d->done);
    munmap(d, sizeof(*d));
    shm_unlink("/data.shm");
}
