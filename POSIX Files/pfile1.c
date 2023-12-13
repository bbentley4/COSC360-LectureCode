#include <stdio.h>
#include <fcntl.h> // open(), #defines
#include <unistd.h> // read(), write()
int main(int argc, char *argv[])
{
    int fd;
    ssize_t bytes;
    char buffer[64];
    if (argc < 2) 
    {
        printf("Usage: %s <file name>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd < 0) 
    {
        perror("open");
        return -1;
    }

    bytes = read(fd, buffer, 63);

    if (bytes < 0) 
    {
        perror("read");
        close(fd);
        return -1;
    }
    
    close(fd);
    printf("Read: '%.*s'\n", (int)bytes, buffer);
    return 0;
}
