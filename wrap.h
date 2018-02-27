#ifndef _WRAP_H_
#define _WRAP_H_

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

void PerrAndExit(const char *remind);

int Socket(int family, int type, int protocol);

void Bind(int fd, const struct sockaddr *sa, socklen_t sa_len);

void Listen(int fd, int backlog);

int Accept(int fd, struct sockaddr *sa, socklen_t *sa_len);

void Connect(int fd, const struct sockaddr *sa, socklen_t sa_len);

ssize_t Recv(int fd, void *buf, ssize_t len, int flags);

ssize_t Send(int fd, const void *buf, ssize_t len, int flags);

ssize_t Read(int fd, void *ptr, size_t nbytes);

ssize_t Write(int fd, const void *vptr, size_t nbytes);

void Close(int fd);

ssize_t ReadN(int fd, const void *vptr, size_t n);

ssize_t WriteN(int fd, const void *vptr, size_t n);

static ssize_t MyRead(int fd, char *ptr) {
    static int read_cnt;
    static char *read_ptr;
    static char read_buf[100];

    if (read_cnt <= 0) {
again:
        if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR) goto again;
            else return -1;
        }
        else if (read_cnt == 0) {
            return 0;
        }

        read_ptr = read_buf;
    }
    --read_cnt;
    *ptr = *read_ptr++;

    return 1;
}

size_t ReadLine(int fd, void *vptr, ssize_t max_len);


#endif /* _WRAP_H_ */
