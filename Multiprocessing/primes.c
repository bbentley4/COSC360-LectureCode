// Demonstrates using message queues to
// command a thread to calculate a range of
// primes.
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mqueue.h>
#define NUM_THREADS 5
#define PRIME_SPAN 10

typedef struct {
    // Find primes between [start..end]
    unsigned int start;
    unsigned int end;
} Command;

bool test_prime(unsigned int value)
{
    unsigned int i;
    for (i = 2;i * i <= value;i+=1) 
    {
        if ((value % i) == 0) 
        {
            return false;
        }
    }
    return true;
}

int thread_worker(void)
{
    mqd_t mqueue;
    char *message;
    struct mq_attr attr;
    unsigned int priority = 0;
    Command *cmd;
    unsigned int i;
    mqueue = mq_open("/prime.mq", O_RDONLY);

    if (mqueue < 0) 
    {
        perror("child mq_open");
        return 1;
    }

    if (mq_getattr(mqueue, &attr) < 0) 
    {
        return 1;
    }
    message = (char *)calloc(attr.mq_msgsize, sizeof(char));
    cmd = (Command *)message;

    // The message is bigger than a command, but we only care about the command.
    if ((mq_receive(mqueue, message, attr.mq_msgsize, &priority)) < 0) 
    {
        perror("mq_receive");
        return 1;
    }

    mq_close(mqueue);
    for (i = cmd->start;i <= cmd->end;i += 1) 
    {
        if (test_prime(i)) 
        {
            printf("(child %6d): %6d is prime!\n", getpid(), i);
        }
    }
    free(message);
    return 0;
}

int main(void)
{
    int i;
    mqd_t mqueue;
    Command cmd;
    pid_t *pids = calloc(NUM_THREADS, sizeof(pid_t));
    mqueue = mq_open("/prime.mq", O_CREAT | O_EXCL | O_WRONLY, 0600, NULL);
    if (mqueue < 0) 
    {
        perror("parent mq_open");
        return -1;
    }
    for (i = 0;i < NUM_THREADS;i++) 
    {
        if ((pids[i] = fork()) == 0) 
        {
            exit(thread_worker());
        }
    }

    // We do this so we can demonstrate message queues
    for (i = 0;i < NUM_THREADS;i++) 
    {
        cmd.start = i * PRIME_SPAN + 2;
        cmd.end = cmd.start + (PRIME_SPAN - 1);
        mq_send(mqueue, (char*)&cmd, sizeof(cmd), 1);
    }

    for (i = 0;i < NUM_THREADS;i++) 
    {
        waitpid(pids[i], NULL, 0);
    }
    mq_close(mqueue);
    mq_unlink("/prime.mq");
    free(pids);
    return 0;
}
