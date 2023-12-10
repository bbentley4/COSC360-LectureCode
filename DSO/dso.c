#include <stddef.h> // NULL
struct export {
    double (*ptr)(double, double, double);
};

double action(double a, double b, double c)
{
    return a * b + c;
}
struct export exports[] = {
    { action },
    { NULL },
};

