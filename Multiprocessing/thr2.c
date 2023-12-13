#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
int main() 
{
    const int BYTES = 64;
    char *mem;
    int fd;
    fd = shm_open("/test.shm", O_RDWR, 0);
    if (fd < 0) 
    {
        perror("shm_open");
        return -1;
    }
    mem = mmap(NULL, BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (!mem) 
    {
        perror("mmap");
        return -1;
    }
    close(fd);
    printf("%d %d %d\n", mem[0], mem[1], mem[2]);
    munmap(mem, BYTES);
    shm_unlink("/test.shm");
}
