#ifndef _WRAP_H_
#define _WRAP_H_

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

void setreuse(SOCKET fd);
void setnonblock(SOCKET fd);
void perr_exit(const char *s);
SOCKET Accept(SOCKET fd, struct sockaddr *sa, int *salenptr);
int Bind(SOCKET fd, const struct sockaddr *sa, int salen);
int Connect(SOCKET fd, const struct sockaddr *sa, int salen);
int Listen(SOCKET fd, int backlog);
SOCKET Socket(int family, int type, int protocol);
int Read(SOCKET fd, void *ptr, int nbytes);
int Write(SOCKET fd, const void *ptr, int nbytes);
int Close(SOCKET fd);
int Readn(SOCKET fd, void *vptr, int n);
int Writen(SOCKET fd, const void *vptr, int n);
int Readline(SOCKET fd, void *vptr, int maxlen);

#else
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <fcntl.h>

void settcpnodelay(int fd);
void setreuse(int fd);
void setnonblock(int fd);
void perr_exit(const char *s);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
int Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
int Close(int fd);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
static ssize_t my_read(int fd, char *ptr);
ssize_t Readline(int fd, void *vptr, size_t maxlen);

#endif

#endif
