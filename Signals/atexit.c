#include <stdio.h>
#include <unistd.h> // alarm(), sleep()
#include <signal.h> // signal()
#include <stdlib.h> // atexit()

void handler(int sig)
{
    (void)sig;
    puts("In alarm handler...sleeping before exiting.");
    sleep(1);
    exit(EXIT_SUCCESS);
}

void do_exit(void)
{
    puts("In atexit handler...bye...");
}

int main(void)
{
    atexit(do_exit);
    signal(SIGALRM, handler);
    alarm(2);
    puts("Alarm set, going to sleep...");
    do {
        sleep(1);
    } while (1);
}
