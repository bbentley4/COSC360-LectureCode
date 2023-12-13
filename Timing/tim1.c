#include <time.h>
#include <stdio.h>
int main(void) {
time_t t1;
time_t t2;
struct tm tm1;
struct tm tm2;
struct timespec ts;
char output[256];
// One way to get UNIX time
t1 = time(NULL);
// Sleep for 1 second.
ts.tv_sec = 1;
ts.tv_nsec = 0;
nanosleep(&ts, NULL);
// Another way
time(&t2);
// Convert time_t (t1) to struct tm (tm1).
localtime_r(&t1, &tm1);
strftime(output, sizeof(output), "%H:%M:%S %d/%m/%Y%n", &tm1);
printf("%s", output);
// Try UTC time instead of local time zone
gmtime_r(&t2, &tm2);
strftime(output, sizeof(output), "%h:%M:%S %D%n", &tm2);
printf("%s", output);
printf("Time difference = %lf seconds.\n", difftime(t2, t1));
}
