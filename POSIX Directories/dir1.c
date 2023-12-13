#include <stdio.h>
#include <unistd.h>
#include <dirent.h> // *dir()
#include <stddef.h> // NULL

int main(int argc, char *argv[])
{
    DIR *dir;
    struct dirent *dent;
    if (argc < 2) 
    {
        printf("Usage: %s <directory>\n", argv[0]);
        return -1;
    }
    dir = opendir(argv[1]);

    if (!dir) 
    {
        perror("opendir");
        return -1;
    }

    while ((dent = readdir(dir)) != NULL) 
    {
        printf("Entry: %s\n", dent->d_name);
    }
    
    closedir(dir);
    return 0;
}