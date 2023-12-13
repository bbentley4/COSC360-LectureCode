#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h> // alarm()

void handler(int sig)
{
    // Turn off "unused" warning
    (void)sig;
    puts("Ok, ok, I'll let you go.");
    exit(EXIT_SUCCESS);
}

int main(void)
{
    signal(SIGALRM, handler);
    alarm(1);
    // This will infinitely output the text, but the alarm is
    // scheduled 1 second into the future, which will cause
    // control to jump to handler() above.
    do 
    {
        puts("I'm not letting you go (press CTRL-C to quit): ");
    } while (1);
}
