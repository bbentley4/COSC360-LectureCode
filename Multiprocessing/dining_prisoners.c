#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
// #define UNDO_LOCKS

#define SHM_FILE "dprison.shm"
#define NUM_PRISONERS 5
#define EAT_TIME (1234 + rand() % 7199)
#define STACK_SIZE 1024

struct Shared {
    sem_t forks[5];
};

static int do_child(void);
int main(void)
{
    int shmfd;
    int i;
    int status;
    pid_t children[NUM_PRISONERS];
    struct Shared *sh;
    srand(time(NULL));
    shmfd = shm_open(SHM_FILE, O_CREAT | O_EXCL | O_RDWR, 0600);

    if (shmfd < 0) 
    {
        perror("parent shm_open");
        return -1;
    }

    ftruncate(shmfd, sizeof(struct Shared));

    for (i = 0;i < NUM_PRISONERS;i+=1) 
    {
        pid_t pid = fork();
        if (pid > 0) 
        {
            children[i] = fork();
        }
        else if (pid == 0) 
        {
            close(shmfd);
            int ret = do_child();
            exit(ret);
        }
        else 
        {
            perror("fork");
            goto shm_cleanup;
            return -1;
        }
    }

    sh = mmap(NULL, sizeof(struct Shared), PROT_READ | PROT_WRITE, MAP_SHARED,
    shmfd, 0);
    close(shmfd);

    // Join children
    for (i = 0;i < NUM_PRISONERS;i+=1) 
    {
        waitpid(children[i], NULL, 0);
    }

    munmap(sh, sizeof(struct Shared));
    shm_cleanup:
    shm_unlink(SHM_FILE);
    return 0;
}
    
static int do_child(void)
{
    struct Shared *sh;
    int shmfd;
    shmfd = shm_open(SHM_FILE, O_RDWR, 0600);
    if (shmfd < 0) 
    {
        perror("child shm_open");
        return 1;
    }
    sh = mmap(NULL, sizeof(struct Shared), PROT_READ | PROT_WRITE, MAP_SHARED,
    shmfd, 0);
    close(shmfd);
    munmap(sh, sizeof(struct Shared));
    return 0;
}
