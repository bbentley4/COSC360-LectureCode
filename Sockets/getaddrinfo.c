#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#define HOST "web.eecs.utk.edu"
#define PORT "80"
#define URL "/~smarz1/courses/cosc360/index.htm"
int main(void)
{
int fd;
int r;
int len;
char buf[4096];
struct addrinfo hints;
struct addrinfo *result, *p;
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;
hints.ai_protocol = 0;
hints.ai_canonname = NULL;
hints.ai_addr = NULL;
hints.ai_next = NULL;
r = getaddrinfo(HOST, PORT, &hints, &result);
if (r != 0) {
printf("Error with %s:%s - %s\n", HOST, PORT, gai_strerror(r));
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
printf("Invalid address %s:%s\n", HOST, PORT);
return -1;
}
snprintf(buf, 4096, "GET %.64s HTTP/1.1\r\nHost: %.46s\r\nConnection: close\r\
n\r\n", URL, HOST);
send(fd, buf, strlen(buf), 0);
while ((len = recv(fd, buf, sizeof(buf)-1, 0)) > 0) {
buf[len] = '\0';
printf("%s", buf);
}
puts("");
shutdown(fd, SHUT_RDWR);
close(fd);
}
