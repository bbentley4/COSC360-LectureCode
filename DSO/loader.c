#include <stdio.h>
#include <dlfcn.h>

struct export {
    double (*ptr)(double, double, double);
};
int main(void)
{
    void *handle;
    struct export *export;
    handle = dlopen("./libdso.so", RTLD_LAZY);
    if (!handle) {
        // Can't use perror here.
        puts(dlerror());
        return -1;
    }
    export = dlsym(handle, "exports");
    double a = 10;
    double b = 0.33;
    double c = 3;

    printf("Value = %lf.\n", export->ptr(a, b, c));
    
    dlclose(handle);
}
