#include <stdio.h>
#include <unistd.h> // fork()
#include <sys/wait.h> // waitpid()
int i;
int main(void)
{
    pid_t pid;
    i = 22;
    pid = fork();
    if (pid == 0) 
    {
        // Child gets 0
        i = 33;
    }
    else 
    {
        // Parent gets child's PID
        i = 44;
        // Wait on the child to finish
        waitpid(pid, NULL, 0);
    }
    // Both the child and parent will get here
    // fork() copies on the first write, so the child
    // will print 33 and the parent will write 44.
    printf("i = %d\n", i);
}
