//
// Created by wangj on 2020/6/2.
//

#ifndef HSERVER_WARP_H
#define HSERVER_WARP_H

#include <unistd.h>

void error_exit(const char *msg);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Listen(int sockfd, int backlog);

int Socket(int domain, int type, int protocol);

ssize_t Read(int fd, void *buf, size_t count);

ssize_t Write(int fd, const void *buf, size_t count);

int Close(int fd);

ssize_t Readn(int fd, void *vptr, size_t n);

ssize_t Writen(int fd, const void *vptr, size_t n);

ssize_t my_read(int fd, char *ptr);

ssize_t Readline(int fd, void *vptr, size_t maxlen);

int Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

int Fcntl(int fd, int cmd, int arg);

#endif //HSERVER_WARP_H
