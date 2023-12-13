#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handler(int sig)
{
    // Turn off "unused" warning
    (void)sig;
    puts("Ok, ok, I'll let you go.");
    exit(EXIT_SUCCESS);
}

int main(void)
{
    signal(SIGINT, handler);
    do 
    {
        puts("I'm not letting you go (press CTRL-C to quit): ");
        getchar();
    } while (1);
}
