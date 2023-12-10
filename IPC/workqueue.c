// Message queues doing work in a thread pool
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mqueue.h>

#define THREAD_POOL_SIZE    5
#define NUM_CALCULATIONS    25
#define MQ_NAME             "/tp_workqueue.mq"

enum Command {
    COMMAND_WORK, // Do work
    COMMAND_DIE, // Close the thread
};

struct Work {
    enum Command cmd;
    float input;
};

int thread_worker(void)
{
    mqd_t mqueue;
    char *message;
    struct mq_attr attr;
    unsigned int priority = 0;
    struct Work *work;
    mqueue = mq_open(MQ_NAME, O_RDONLY);

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
    work = (struct Work *)message;

    do 
    {
        // The message is bigger than a command, but we only care about the
        command.
        if ((mq_receive(mqueue, message, attr.mq_msgsize, &priority)) < 0) 
        {
            perror("mq_receive");
            break;
        }
        if (work->cmd == COMMAND_DIE) 
        {
            break;
        }
        else if (work->cmd == COMMAND_WORK) 
        {
            printf("(worker #%d): %f * %f = %f.\n", getpid(), work->input, work->input, work->input * work->input);
        }
    } while (1);

    mq_close(mqueue);
    free(message);
    return 0;
}
int main(void)
{
    int i;
    mqd_t mqueue;
    struct Work work;
    pid_t *pids = calloc(THREAD_POOL_SIZE, sizeof(pid_t));
    mqueue = mq_open(MQ_NAME, O_CREAT | O_EXCL | O_WRONLY, 0600, NULL);
    
    if (mqueue < 0) 
    {
        perror("parent mq_open");
        return -1;
    }
    for (i = 0;i < THREAD_POOL_SIZE;i++) 
    {
        if ((pids[i] = fork()) == 0) 
        {
            exit(thread_worker());
        }
    }

    // We do this so we can demonstrate message queues
    for (i = 0;i < NUM_CALCULATIONS;i++) 
    {
        work.cmd = COMMAND_WORK;
        work.input = i + 1;
        mq_send(mqueue, (char*)&work, sizeof(work), 1);
    }
    // Send the die message
    for (i = 0;i < THREAD_POOL_SIZE;i++) 
    {
        work.cmd = COMMAND_DIE;
        mq_send(mqueue, (char*)&work, sizeof(work), 1);
    }
    // Join the threads
    for (i = 0;i < THREAD_POOL_SIZE;i++) 
    {
        waitpid(pids[i], NULL, 0);
    }
    mq_close(mqueue);
    mq_unlink(MQ_NAME);
    free(pids);
    return 0;
}
