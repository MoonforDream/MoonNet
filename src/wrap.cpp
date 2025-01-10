#ifdef _WIN32

#include "wrap.h"
#include <cstdio>
#include <cstring>

void setreuse(SOCKET fd) {
    int opt = 1;
    int ret =
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    if (SOCKET_ERROR == ret) {
        perr_exit("setsockopt ipreuse error");
    }
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt));
    if (SOCKET_ERROR == ret) {
        perr_exit("setsockopt portreuse error");
    }
}

void setnonblock(SOCKET fd) {
    u_long mode = 1;
    ioctlsocket(fd, FIONBIO, &mode)
    //    if (ioctlsocket(fd, FIONBIO, &mode) != 0) {
    //        perror("ioctlsocket failed");
    //    }
}

void perr_exit(const char *s) {
    perror(s);
    WSACleanup();
    exit(1);
}

SOCKET Accept(SOCKET fd, struct sockaddr *sa, int *salenptr) {
    SOCKET n;
again:
    if ((n = accept(fd, sa, salenptr)) == INVALID_SOCKET) {
        if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAEINTR)
            goto again;
        else
            perr_exit("accept error");
    }
    return n;
}

int Bind(SOCKET fd, const struct sockaddr *sa, int salen) {
    if (bind(fd, sa, salen) == SOCKET_ERROR) {
        perr_exit("bind error");
    }
    return 0;
}

int Connect(SOCKET fd, const struct sockaddr *sa, int salen) {
    if (connect(fd, sa, salen) == SOCKET_ERROR) {
        perr_exit("connect error");
    }
    return 0;
}

int Listen(SOCKET fd, int backlog) {
    if (listen(fd, backlog) == SOCKET_ERROR) {
        perr_exit("listen error");
    }
    return 0;
}

SOCKET Socket(int family, int type, int protocol) {
    SOCKET n;
    if ((n = socket(family, type, protocol)) == INVALID_SOCKET) {
        perr_exit("socket error");
    }
    return n;
}

int Read(SOCKET fd, void *ptr, int nbytes) {
    int n;
    if ((n = recv(fd, (char *)ptr, nbytes, 0)) == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINTR)
            return Read(fd, ptr, nbytes);
        else
            return -1;
    }
    return n;
}

int Write(SOCKET fd, const void *ptr, int nbytes) {
    int n;
    if ((n = send(fd, (char *)ptr, nbytes, 0)) == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEINTR)
            return Write(fd, ptr, nbytes);
        else
            return -1;
    }
    return n;
}

int Close(SOCKET fd) {
    if (closesocket(fd) == SOCKET_ERROR) {
        perr_exit("close error");
    }
    return 0;
}

int Readn(SOCKET fd, void *vptr, int n) {
    int nleft, nread;
    char *ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nread = recv(fd, ptr, nleft, 0)) < 0) {
            if (WSAGetLastError() == WSAEINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0)
            break;  // EOF

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);
}

int Writen(SOCKET fd, const void *vptr, int n) {
    int nleft, nwritten;
    const char *ptr = (const char *)vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = send(fd, ptr, nleft, 0)) <= 0) {
            if (nwritten < 0 && WSAGetLastError() == WSAEINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

int Readline(SOCKET fd, void *vptr, int maxlen) {
    int n, rc;
    char c, *ptr;
    ptr = (char *)vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = Read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n') break;
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);
        } else
            return -1;  // Error
    }
    *ptr = 0;
    return n;
}

#else

#include "wrap.h"

/**
 * @brief Sets the TCP_NODELAY option on a socket.
 *
 * This function disables the Nagle's algorithm for the specified socket,
 * allowing small packets to be sent immediately without waiting for the
 * accumulation of more data.
 *
 * @param fd The file descriptor of the socket on which to set the TCP_NODELAY
 * option.
 */
void settcpnodelay(int fd) {
    int opt = 1;
    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    if (-1 == ret) {
        perr_exit("setsockopt tcp nodelay error");
    }
}

/**
 * @brief Sets the SO_REUSEADDR and SO_REUSEPORT options on a socket.
 *
 * This function allows the socket to be bound to an address that is already in
 * use. It enables the reuse of local addresses and ports, which is useful for
 * server applications that need to restart without waiting for the OS to
 * release the previous socket.
 *
 * @param fd The file descriptor of the socket on which to set the options.
 */
void setreuse(int fd) {
    int opt = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (-1 == ret) {
        perr_exit("setsockopt ipreuse error");
    }
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    if (-1 == ret) {
        perr_exit("setsockopt portreuse error");
    }
}

/**
 * @brief Sets a socket to non-blocking mode.
 *
 * This function modifies the file descriptor flags to enable non-blocking I/O
 * operations on the specified socket. In non-blocking mode, I/O operations will
 * return immediately without waiting, which is essential for event-driven
 * programming.
 *
 * @param fd The file descriptor of the socket to be set to non-blocking mode.
 */
void setnonblock(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

/**
 * @brief Prints an error message and exits the program.
 *
 * This function is used to display an error message corresponding to the
 * current value of `errno` and terminate the program with a failure status.
 *
 * @param s A C-string describing the context of the error.
 */
void perr_exit(const char *s) {
    perror(s);
    exit(1);
}

/**
 * @brief Accepts a new connection on a listening socket.
 *
 * This function wraps the `accept` system call, handling interrupted system
 * calls and retrying if necessary. It returns the file descriptor for the
 * accepted connection or -1 if no connections are available to accept.
 *
 * @param fd The file descriptor of the listening socket.
 * @param sa A pointer to a `sockaddr` structure that will be filled with the
 * address of the connecting entity.
 * @param salenptr A pointer to a `socklen_t` variable that initially contains
 * the size of the `sa` structure and will be updated to indicate the actual
 * size of the address returned.
 *
 * @return On success, returns the file descriptor for the accepted socket.
 *         On failure, returns -1 if no connections are available or terminates
 * the program on other errors.
 */
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    int n;
again:
    if ((n = accept(fd, sa, salenptr)) < 0) {
        // ECONNABORTED表示连接被本地软件意外中断
        // EINTR表示一个被阻塞的系统调用(如read,write,accept,connect等)被signal打断了
        if ((errno == ECONNABORTED) || (errno == EINTR))
            goto again;
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
            return -1;  // 暂时没有新的连接请求到达，返回-1
        else
            perr_exit("accept error");
    }
    return n;
}

/**
 * @brief Accepts a new connection on a listening socket.
 *
 * This function wraps the `accept` system call, handling interrupted system
 * calls and retrying if necessary. It returns the file descriptor for the
 * accepted connection or -1 if no connections are available to accept.
 *
 * @param fd The file descriptor of the listening socket.
 * @param sa A pointer to a `sockaddr` structure that will be filled with the
 * address of the connecting entity.
 * @param salenptr A pointer to a `socklen_t` variable that initially contains
 * the size of the `sa` structure and will be updated to indicate the actual
 * size of the address returned.
 *
 * @return On success, returns the file descriptor for the accepted socket.
 *         On failure, returns -1 if no connections are available or terminates
 * the program on other errors.
 */
int Bind(int fd, const struct sockaddr *sa, socklen_t salen) {
    int n;

    if ((n = bind(fd, sa, salen)) < 0) {
        perr_exit("bind error");
    }

    return n;
}

/**
 * @brief Connects a socket to a remote address.
 *
 * This function wraps the `connect` system call, establishing a connection to
 * the address specified by `sa` for the socket represented by `fd`. It exits
 * the program on failure.
 *
 * @param fd The file descriptor of the socket to be connected.
 * @param sa A pointer to a `sockaddr` structure containing the address to
 * connect to.
 * @param salen The size of the address structure pointed to by `sa`.
 *
 * @return On success, returns 0.
 *         On failure, returns -1 after exiting the program.
 */
int Connect(int fd, const struct sockaddr *sa, socklen_t salen) {
    int n;
    if ((n = connect(fd, sa, salen)) < 0) {
        perr_exit("connect error");
        return -1;
    }
    return n;
}

/**
 * @brief Listens for incoming connections on a socket.
 *
 * This function wraps the `listen` system call, marking the socket referred to
 * by `fd` as a passive socket that will be used to accept incoming connection
 * requests. It exits the program on failure.
 *
 * @param fd The file descriptor of the socket to listen on.
 * @param backlog The maximum length to which the queue of pending connections
 * may grow.
 *
 * @return On success, returns 0.
 *         On failure, returns -1 after exiting the program.
 */
int Listen(int fd, int backlog) {
    int n;

    if ((n = listen(fd, backlog)) < 0) {
        perr_exit("listen error");
        return -1;
    }

    return n;
}

/**
 * @brief Creates a new socket.
 *
 * This function wraps the `socket` system call, creating a new socket of the
 * specified family, type, and protocol. It exits the program on failure.
 *
 * @param family The communication domain (e.g., `AF_INET` for IPv4).
 * @param type The socket type (e.g., `SOCK_STREAM` for TCP).
 * @param protocol The protocol to be used with the socket (usually 0 to select
 * the default).
 *
 * @return On success, returns the file descriptor for the new socket.
 *         On failure, returns -1 after exiting the program.
 */
int Socket(int family, int type, int protocol) {
    int n;
    if ((n = socket(family, type, protocol)) < 0) {
        perr_exit("socket error");
        return -1;
    }

    return n;
}

/**
 * @brief Creates a new socket.
 *
 * This function wraps the `socket` system call, creating a new socket of the
 * specified family, type, and protocol. It exits the program on failure.
 *
 * @param family The communication domain (e.g., `AF_INET` for IPv4).
 * @param type The socket type (e.g., `SOCK_STREAM` for TCP).
 * @param protocol The protocol to be used with the socket (usually 0 to select
 * the default).
 *
 * @return On success, returns the file descriptor for the new socket.
 *         On failure, returns -1 after exiting the program.
 */
ssize_t Read(int fd, void *ptr, size_t nbytes) {
    ssize_t n;

again:
    if ((n = read(fd, ptr, nbytes)) == -1) {
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }

    return n;
}

/**
 * @brief Creates a new socket.
 *
 * This function wraps the `socket` system call, creating a new socket of the
 * specified family, type, and protocol. It exits the program on failure.
 *
 * @param family The communication domain (e.g., `AF_INET` for IPv4).
 * @param type The socket type (e.g., `SOCK_STREAM` for TCP).
 * @param protocol The protocol to be used with the socket (usually 0 to select
 * the default).
 *
 * @return On success, returns the file descriptor for the new socket.
 *         On failure, returns -1 after exiting the program.
 */
ssize_t Write(int fd, const void *ptr, size_t nbytes) {
    ssize_t n;

again:
    if ((n = write(fd, ptr, nbytes)) == -1) {
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }

    return n;
}

/**
 * @brief Creates a new socket.
 *
 * This function wraps the `socket` system call, creating a new socket of the
 * specified family, type, and protocol. It exits the program on failure.
 *
 * @param family The communication domain (e.g., `AF_INET` for IPv4).
 * @param type The socket type (e.g., `SOCK_STREAM` for TCP).
 * @param protocol The protocol to be used with the socket (usually 0 to select
 * the default).
 *
 * @return On success, returns the file descriptor for the new socket.
 *         On failure, returns -1 after exiting the program.
 */
int Close(int fd) {
    int n;
    if ((n = close(fd)) == -1) perr_exit("close error");
    return n;
}

/**
 * @brief Creates a new socket.
 *
 * This function wraps the `socket` system call, creating a new socket of the
 * specified family, type, and protocol. It exits the program on failure.
 *
 * @param family The communication domain (e.g., `AF_INET` for IPv4).
 * @param type The socket type (e.g., `SOCK_STREAM` for TCP).
 * @param protocol The protocol to be used with the socket (usually 0 to select
 * the default).
 *
 * @return On success, returns the file descriptor for the new socket.
 *         On failure, returns -1 after exiting the program.
 */
// 参数三：是应该读取的字节数
ssize_t Readn(int fd, void *vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = (char *)vptr;
    nleft = n;

    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0) {
            break;
        }

        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

/**
 * @brief Writes exactly `n` bytes to a file descriptor.
 *
 * This function attempts to write `n` bytes from the buffer pointed to by
 * `vptr` to the file descriptor `fd`. It continues writing until either all `n`
 * bytes have been written or an error occurs.
 *
 * @param fd The file descriptor to write to.
 * @param vptr A pointer to the buffer containing the data to be written.
 * @param n The exact number of bytes to write.
 *
 * @return On success, returns the number of bytes written (which will be equal
 * to `n`). On failure, returns -1.
 */
ssize_t Writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

/**
 * @brief Reads a single character from a file descriptor.
 *
 * This static helper function reads one character at a time from the file
 * descriptor `fd`. It buffers the input to minimize the number of system calls.
 *
 * @param fd The file descriptor to read from.
 * @param ptr A pointer to the buffer where the read character will be stored.
 *
 * @return On success, returns 1.
 *         On end of file, returns 0.
 *         On error, returns -1.
 */
static ssize_t my_read(int fd, char *ptr) {
    static int read_cnt;
    static char *read_ptr;
    static char read_buf[100];

    if (read_cnt <= 0) {
    again:
        if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR) goto again;
            return -1;
        } else if (read_cnt == 0)
            return 0;
        read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

/**
 * @brief Reads a single character from a file descriptor.
 *
 * This static helper function reads one character at a time from the file
 * descriptor `fd`. It buffers the input to minimize the number of system calls.
 *
 * @param fd The file descriptor to read from.
 * @param ptr A pointer to the buffer where the read character will be stored.
 *
 * @return On success, returns 1.
 *         On end of file, returns 0.
 *         On error, returns -1.
 */
// readline ---fgets
// 传出参数 vptr
ssize_t Readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;
    ptr = (char *)vptr;

    for (n = 1; n < maxlen; ++n) {
        if ((rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n') break;
        } else if (rc == 0) {
            *ptr = 0;
            return n - 1;
        } else {
            return -1;
        }
    }
    *ptr = 0;
    return n;
}
#endif
