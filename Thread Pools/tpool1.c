#include <semaphore.h>
#include <mqueue.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

#define MQD_FILENAME "/tpool1.mqd"
#define SHM_FILENAME "/tpool1.shm"
#define NUM_THREADS 5
#define STEP_VALUE 0.3125
#define NUM_UPDATES 1000

typedef struct {
    int update;
    int step_value;
} Work;

typedef struct {
    double result;
    sem_t wrlock;
    sem_t rdlock;
    int num_readers;
} Shared;

static int do_child(void);

int main(void)
{
    int i;
    int shm;
    mqd_t mq;
    pid_t threads[NUM_THREADS];
    Shared *s;

    mq = mq_open(MQD_FILENAME, O_CREAT | O_EXCL | O_WRONLY, 0600, NULL);
    if (mq < 0) 
    {
        perror("mq_open");
        return -1;
    }
    shm = shm_open(SHM_FILENAME, O_CREAT | O_EXCL | O_RDWR, 0600);

    if (shm < 0) {
        perror("shm_open");
        goto mq_cleanup;
    }

    ftruncate(shm, sizeof(Shared));
    s = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (s == MAP_FAILED) 
    {
        perror("mmap");
        goto shm_cleanup;
    }
    close(shm);

    s->result = 0;
    s->num_readers = 0;
    sem_init(&s->wrlock, 1, 1);
    sem_init(&s->rdlock, 1, 1);

    for (i = 0;i < NUM_THREADS;i+=1) 
    {
        pid_t pid = fork();
        if (pid < 0) 
        {
            perror("fork");
            break;
        }
        else if (pid == 0) 
        {
            mq_close(mq);
            munmap(s, sizeof(Shared));
            exit(do_child());
        }
        threads[i] = pid;
    }

    for (i = 0;i < NUM_THREADS;i+=1) 
    {
        waitpid(threads[i], NULL, 0);
    }
    sem_destroy(&s->rdlock);
    sem_destroy(&s->wrlock);
    printf("Final result: %lf\n", s->result);
    munmap(s, sizeof(Shared));
    shm_cleanup:
    shm_unlink(SHM_FILENAME);
    mq_cleanup:
    mq_close(mq);
    mq_unlink(MQD_FILENAME);
}

static double get_result(Shared *s)
{
    double ret;
    sem_wait(&s->rdlock);
    s->num_readers += 1;
    if (s->num_readers == 1) 
    {
        sem_wait(&s->wrlock);
    }
    sem_post(&s->rdlock);
    ret = s->result;
    sem_wait(&s->rdlock);
    s->num_readers -= 1;

    if (s->num_readers == 0) 
    {
        sem_post(&s->wrlock);
    }
    sem_post(&s->rdlock);
    return ret;
}

static double add_to_result(Shared *s, double value)
{
    double ret;
    sem_wait(&s->wrlock);
    ret = s->result += value;
    sem_post(&s->wrlock);
    return ret;
}

static int do_child(void)
{
    double r;
    int shm = shm_open(SHM_FILENAME, O_RDWR, 0600);
    if (shm < 0) 
    {
        perror("shm_open");
        return -1;
    }
    Shared *s = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (s == MAP_FAILED) 
    {
        perror("mmap");
        return -1;
    }
    close(shm);
    r = get_result(s);
    printf("In %d: Result = %lf.\n", getpid(), r);
    r = add_to_result(s, 100);
    printf("After %d: Result = %lf.\n", getpid(), r);
    munmap(s, sizeof(Shared));
    return 0;
}
