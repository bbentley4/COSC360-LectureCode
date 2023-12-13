#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "vector.h"
typedef enum {
STATE_WORK,
STATE_CLOSE
} State;
typedef struct {
int fd;
pthread_t tid;
State state;
} Thread;
int g_server;
Vector *g_clients;
static void cleanup(void);
static void sighandler(int sig);
static void *worker(void *);
static void setblocking(int fd, bool block);
int main(int argc, char *argv[])
{
unsigned short port;
struct sockaddr_in6 sin6;
socklen_t sin6len;
int fd;
int i;
Thread *t;
if (argc < 2) {
printf("Usage: %s <port>\n", argv[0]);
return -1;
}
if ((sscanf(argv[1], "%hu", &port)) != 1) {
printf("Invalid port '%s'\n", argv[1]);
return -1;
}
g_server = socket(AF_INET6, SOCK_STREAM, 0);
if (g_server <= 0) {
perror("socket");
return -1;
}
memset(&sin6, 0, sizeof(sin6));
sin6.sin6_family = AF_INET6;
sin6.sin6_port = htons(port);
sin6.sin6_addr = in6addr_loopback;
if (bind(g_server, (struct sockaddr *)&sin6, sizeof(sin6)) != 0) {
perror("bind");
return -1;
}
g_clients = vector_new();
atexit(cleanup);
signal(SIGINT, sighandler);
listen(g_server, 10);
for (;;) {
setblocking(g_server, vector_size(g_clients) == 0);
sin6len = sizeof(sin6);
fd = accept(g_server, (struct sockaddr *)&sin6, &sin6len);
if (fd > 0) {
t = malloc(sizeof(Thread));
t->fd = fd;
t->state = STATE_WORK;
vector_push_ptr(g_clients, t);
pthread_create(&t->tid, NULL, worker, t);
}
for (i = 0;i < vector_size(g_clients);i+=1) {
t = vector_get_unchecked_ptr(g_clients, i);
if (t->state == STATE_CLOSE) {
pthread_join(t->tid, NULL);
close(t->fd);
vector_remove(g_clients, i);
free(t);
i -= 1;
}
}
}
}
static void *worker(void *arg)
{
Thread *me = (Thread *)arg;
fd_set rds;
int nfds;
char buf[256];
ssize_t bytes;
for (;;) {
if (me->state == STATE_CLOSE) {
break;
}
FD_ZERO(&rds);
FD_SET(me->fd, &rds);
nfds = select(me->fd + 1, &rds, NULL, NULL, &(struct timeval){0, 100});
if (nfds > 0 && FD_ISSET(me->fd, &rds)) {
bytes = recv(me->fd, buf, 255, 0);
if (bytes <= 0) {
me->state = STATE_CLOSE;
break;
}
send(me->fd, buf, bytes, 0);
}
}
return NULL;
}
static void cleanup(void)
{
int i;
Thread *t;
for (i = 0; i < vector_size(g_clients); i += 1) {
t = vector_get_unchecked_ptr(g_clients, i);
t->state = STATE_CLOSE;
pthread_join(t->tid, NULL);
close(t->fd);
free(t);
}
vector_free(g_clients);
close(g_server);
}
static void sighandler(int sig)
{
(void)sig;
exit(EXIT_SUCCESS);
}
static void setblocking(int fd, bool block)
{
int fl = fcntl(fd, F_GETFL);
if (block) {
fl &= ~O_NONBLOCK;
}
else {
fl |= O_NONBLOCK;
}
fcntl(fd, F_SETFL, fl);
}
