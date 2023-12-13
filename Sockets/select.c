#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include "vector.h"
Vector *g_client_list;
int g_server;
static void sighandler(int sig)
{
(void)sig;
exit(EXIT_SUCCESS);
}
static void cleanup(void)
{
int i;
int cl;
for (i = 0;i < vector_size(g_client_list);i+=1) {
cl = vector_get_unchecked(g_client_list, i);
close(cl);
}
vector_free(g_client_list);
close(g_server);
}
static void setblocking(int fd, bool block)
{
int flags = fcntl(fd, F_GETFL);
if (block) {
flags &= ~O_NONBLOCK;
}
else {
flags |= O_NONBLOCK;
}
fcntl(fd, F_SETFL, flags);
}
static bool handle_client(int cl)
{
char buffer[256];
ssize_t bytes;
bytes = recv(cl, buffer, 255, 0);
if (bytes <= 0) {
return false;
}
send(cl, buffer, bytes, 0);
return true;
}
int main(int argc, char *argv[])
{
unsigned short port;
struct sockaddr_in sin;
socklen_t len;
int clifd;
int cl;
int i;
fd_set rds;
if (argc < 2) {
printf("Usage: %s <port>\n", argv[0]);
return -1;
}
if (sscanf(argv[1], "%hu", &port) != 1) {
printf("Invalid port specified '%s'\n", argv[1]);
return -1;
}
g_server = socket(AF_INET, SOCK_STREAM, 0);
if (g_server < 0) {
perror("socket");
return -1;
}
g_client_list = vector_new();
atexit(cleanup);
signal(SIGINT, sighandler);
sin.sin_family = AF_INET;
sin.sin_port = htons(port);
sin.sin_addr.s_addr = INADDR_ANY;
if (bind(g_server, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
perror("bind");
return -1;
}
listen(g_server, 10);
for (;;) {
len = sizeof(sin);
clifd = accept(g_server, (struct sockaddr *)&sin, &len);
if (clifd > 0) {
vector_push(g_client_list, clifd);
setblocking(g_server, false);
}
FD_ZERO(&rds);
clifd = 0;
for (i = 0;i < vector_size(g_client_list);i+=1) {
cl = vector_get_unchecked(g_client_list, i);
FD_SET(cl, &rds);
if (cl > clifd) {
clifd = cl;
}
}
if (select(clifd + 1, &rds, NULL, NULL, &(struct timeval){0, 1000}) > 0) {
for (i = 0;i < vector_size(g_client_list);i+=1) {
cl = vector_get_unchecked(g_client_list, i);
if (FD_ISSET(cl, &rds)) {
if (!handle_client(cl)) {
close(cl);
vector_remove(g_client_list, i);
if (vector_size(g_client_list) == 0) {
setblocking(g_server, true);
}
i -= 1;
}
}
}
}
}
return 0;
}
