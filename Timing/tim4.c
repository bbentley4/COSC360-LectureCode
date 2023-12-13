#include <stdio.h>
#include <time.h>
int main(void)
{
time_t st, ed;
st = clock();
for (int i = 0;i < 10000000;i++);
ed = clock();
printf("Total time to run loop: %.0lf clocks (%lu seconds).\n", difftime(ed,
st), (ed - st) / CLOCKS_PER_SEC);
}
