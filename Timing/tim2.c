#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
int main(void) {
nice(5);
long calc = 0;
long tm = time(NULL);
long i;
for (i = 0;i < tm;i++) {
// Just some work to do in the loop.
calc += i * time(NULL) * getpid();
sched_yield();
}
printf("Calc = %ld\n", calc);
}
