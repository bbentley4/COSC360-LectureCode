#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd;
    char buffer1[] = "Hello World!";
    char buffer2[] = "Goodbye";
    if (argc < 2) 
    {
        printf("Usage: %s <file>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_CREAT | O_WRONLY, 0666);

    if (fd < 0) 
    {
        perror("open");
        return -1;
    }

    write(fd, buffer1, strlen(buffer1));
    close(fd);
    fd = open(argv[1], O_WRONLY);

    if (fd < 0) 
    {
        perror("open");
        return -1;
    }
    write(fd, buffer2, strlen(buffer2));
    close(fd);
    // What is in the file?
    return 0;
}
