//
// Created by wangj on 2020/6/2.
//

#include "warp.h"

#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

void error_exit(const char *msg) {
	perror(msg);
	exit(1);
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	int n;
	again:
	if ((n = accept(sockfd, addr, addrlen)) < 0) {
		if ((errno == ECONNABORTED) || (errno == EINTR))
			goto again;
		else
			error_exit("accept error");
	}
	return n;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int n;
	if ((n = bind(sockfd, addr, addrlen)) < 0)
		error_exit("bind error");
	return n;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int n;
	if ((n = connect(sockfd, addr, addrlen)) < 0)
		error_exit("connect error");
	return n;
}

int Listen(int sockfd, int backlog) {
	int n;
	if ((n = listen(sockfd, backlog)) < 0)
		error_exit("listen error");
	return n;
}

int Socket(int domain, int type, int protocol) {
	int n;
	if ((n = socket(domain, type, protocol)) < 0)
		error_exit("socket error");
	return n;
}

ssize_t Read(int fd, void *buf, size_t count) {
	ssize_t n;
	again:
	if ((n = read(fd, buf, count)) == -1) {
		if (errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}

ssize_t Write(int fd, const void *buf, size_t count) {
	ssize_t n;
	again:
	if ((n = write(fd, buf, count)) == -1) {
		if (errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}

int Close(int fd) {
	int n;
	if ((n = close(fd)) == -1)
		error_exit("close error");
	return n;
}

ssize_t Readn(int fd, void *vptr, size_t n) {
	size_t nleft = n;                           //readn函数还需要读的字节数
	ssize_t nread = 0;                          //read函数读到的字节数
	auto *ptr = (unsigned char *) vptr;          //指向缓冲区的指针

	while (nleft > 0) {
		nread = read(fd, ptr, nleft);
		if (-1 == nread) {
			if (EINTR == errno)
				nread = 0;
			else
				return -1;
		} else if (0 == nread) {
			break;
		}
		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n) {
	size_t nleft = n;               //writen函数还需要写的字节数
	ssize_t nwrite = 0;             //write函数本次向fd写的字节数
	const char *ptr = (char *) vptr; //指向缓冲区的指针

	while (nleft > 0) {
		if ((nwrite = write(fd, ptr, nleft)) <= 0) {
			if (nwrite < 0 && EINTR == errno)
				nwrite = 0;
			else
				return -1;
		}
		nleft -= nwrite;
		ptr += nwrite;
	}
	return n;
}

ssize_t my_read(int fd, char *ptr) {
	static int read_cnt;
	static char *read_ptr;
	static char read_buf[100];

	if (read_cnt <= 0) {
		again:
		if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return -1;
		} else if (read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t Readline(int fd, void *vptr, size_t maxlen) {
	ssize_t n, rc;
	char c, *ptr;
	ptr = (char *) vptr;

	for (n = 1; n < maxlen; n++) {
		if ((rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			*ptr = 0;
			return n - 1;
		} else
			return -1;
	}
	*ptr = 0;
	return n;
}

int Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
	int n;
	if ((n = setsockopt(sockfd, level, optname, optval, optlen)) < 0)
		error_exit("setsockfd error");

	return n;
}

int Fcntl(int fd, int cmd, int arg) {
	int n;
	if ((n = fcntl(fd, cmd, arg)) < 0)
		error_exit("fcntl error");

	return n;
}


