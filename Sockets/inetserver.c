#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
int server_fd;
void handler(int sig)
{
(void)sig;
exit(EXIT_SUCCESS);
}
void cleanup(void)
{
shutdown(server_fd, SHUT_RDWR);
close(server_fd);
}
int main(int argc, char *argv[])
{
char data[256];
struct sockaddr_in sin;
int client_fd;
uint16_t port;
pid_t pid;
if (argc < 2) {
printf("Usage: %s <port>\n", argv[0]);
return -1;
}
if (sscanf(argv[1], "%hu", &port) != 1) {
printf("Invalid port specified '%s'\n", argv[1]);
return -1;
}
if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
perror("socket");
return -1;
}
pid = fork();
if (pid < 0) {
perror("fork");
return -1;
}
else if (pid > 0) {
printf("Launching daemon.\n");
printf("Make sure you kill me when you're done!!\n");
printf("~> kill -INT %d\n", pid);
sleep(1);
exit(EXIT_SUCCESS);
}
else {
setsid();
}
sin.sin_family = AF_INET;
sin.sin_port = htons(port); // port is in network byte order!
sin.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 - means accept connections from
any NIC.
if (bind(server_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
perror("bind");
return -1;
}
// Register cleanup since we'll go in an infinite event loop to accept
// connections.
signal(SIGINT, handler);
atexit(cleanup);
listen(server_fd, 10);
while ((client_fd = accept(server_fd, NULL, NULL)) > 0) {
time_t t = time(NULL);
char time_buff[26];
ctime_r(&t, time_buff);
snprintf(data, 256, "Hello, it is %.25s", time_buff);
send(client_fd, data, strlen(data), 0);
close(client_fd);
}
// Exit takes over here
}
