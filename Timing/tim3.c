#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
int main(void)
{
struct stat st;
struct timespec ts = { 0, 1000000 };
while (stat("test.txt", &st) < 0) {
// File does not exist, yet.
// If we left this blank, it would check
// as fast as possible, using up the CPU.
// Adding a delay gives the CPU a breather.
nanosleep(&ts, NULL);
}
printf("test.txt now exists!\n");
}
