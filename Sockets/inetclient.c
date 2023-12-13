#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
int main(int argc, char *argv[])
{
if (argc < 3) {
printf("Usage: %s <address> <port>\n", argv[0]);
return -1;
}
struct addrinfo hints;
struct addrinfo *result, *p;
int fd;
int r;
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;
hints.ai_protocol = 0;
hints.ai_canonname = NULL;
hints.ai_addr = NULL;
hints.ai_next = NULL;
r = getaddrinfo(argv[1], argv[2], &hints, &result);
if (r != 0) {
printf("Error with %s:%s - %s\n", argv[1], argv[2], gai_strerror(r));
return -1;
}
for (p = result;p != NULL;p = p->ai_next) {
fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
if (fd == -1) {
continue;
}
if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
break;
}
// If we get here, it didn't connect.
close(fd);
}
freeaddrinfo(result);
if (p == NULL) {
perror(argv[1]);
return -1;
}
// If we get here, we connected.
char buf[4096];
ssize_t bytes;
bytes = recv(fd, buf, 4095, 0);
if (bytes <= 0) {
printf("Connection closed.\n");
close(fd);
return -1;
}
printf("Server sent '%.*s'\n", (int)bytes, buf);
strcpy(buf, "Hello, I'm a client.\n");
bytes = send(fd, buf, strlen(buf), 0);
if (bytes <= 0) {
printf("I could not send stuff to the server. :(\n");
}
close(fd);
}
