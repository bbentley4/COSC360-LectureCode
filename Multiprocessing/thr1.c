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
    fd = shm_open("/test.shm", O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd < 0) 
    {
        perror("shm_open");
        return -1;
    }
    ftruncate(fd, BYTES);
    mem = mmap(NULL, BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (!mem) 
    {
        perror("mmap");
        return -1;
    }
    close(fd);
    mem[0] = 1;
    mem[1] = 2;
    mem[2] = 3;
    munmap(mem, BYTES);
    shm_unlink("/test.shm");
}
