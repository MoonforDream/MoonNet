#ifdef _WIN32


#include "wrap.h"
#include <cstdio>
#include <cstring>

void setreuse(SOCKET fd){
    int opt=1;
    int ret=setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    if(SOCKET_ERROR==ret){
        perr_exit("setsockopt ipreuse error");
    }
    ret=setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt));
    if(SOCKET_ERROR==ret){
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
            if (c == '\n')
                break;
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);
        } else
            return -1; // Error
    }
    *ptr = 0;
    return n;
}


#else


#include "wrap.h"



void settcpnodelay(int fd){
    int opt=1;
    int ret=setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&opt,sizeof(opt));
    if(-1==ret){
        perr_exit("setsockopt tcp nodelay error");
    }
}


void setreuse(int fd){
    int opt=1;
    int ret=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(-1==ret){
        perr_exit("setsockopt ipreuse error");
    }
    ret=setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt));
    if(-1==ret){
        perr_exit("setsockopt portreuse error");
    }
}

void setnonblock(int fd){
    fcntl(fd,F_SETFL, fcntl(fd,F_GETFL)|O_NONBLOCK);
}

void perr_exit(const char *s){
    perror(s);
    exit(1);
}


int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr){
    int n;
again:
    if((n=accept(fd,sa,salenptr))<0){
        //ECONNABORTED表示连接被本地软件意外中断
        //EINTR表示一个被阻塞的系统调用(如read,write,accept,connect等)被signal打断了
        if((errno==ECONNABORTED)||(errno==EINTR)) goto again;
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
            return -1; // 暂时没有新的连接请求到达，返回-1
        else perr_exit("accept error");
    }
    return n;
}


int Bind(int fd, const struct sockaddr *sa, socklen_t salen){
    int n;

    if((n=bind(fd,sa,salen))<0){
        perr_exit("bind error");
    }

    return n;
}


int Connect(int fd, const struct sockaddr *sa, socklen_t salen){
    int n;
    if((n=connect(fd,sa,salen))<0){
        perr_exit("connect error");
        return -1;
    }
    return n;
}


int Listen(int fd, int backlog){
    int n;

    if((n=listen(fd,backlog))<0){
        perr_exit("listen error");
        return -1;
    }

    return n;
}


int Socket(int family, int type, int protocol){
    int n;
    if((n=socket(family,type,protocol))<0){
        perr_exit("socket error");
        return -1;
    }

    return n;
}


ssize_t Read(int fd, void *ptr, size_t nbytes){
    ssize_t n;

again:
    if((n=read(fd,ptr,nbytes))==-1){
        if(errno==EINTR) goto again;
        else return -1;
    }

    return n;
}


ssize_t Write(int fd, const void *ptr, size_t nbytes){
    ssize_t n;

again:
    if((n=write(fd,ptr,nbytes))==-1){
        if(errno==EINTR) goto again;
        else return -1;
    }

    return n;
}


int Close(int fd){
    int n;
    if((n=close(fd))==-1) perr_exit("close error");
    return n;
}

//参数三：是应该读取的字节数
ssize_t Readn(int fd, void *vptr, size_t n){
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr=(char *)vptr;
    nleft=n;

    while (nleft>0) {
        if((nread=read(fd,ptr,nleft))<0){
            if(errno==EINTR) nread=0;
            else return -1;
        }else if (nread==0) {
            break;
        }

        nleft-=nread;
        ptr+=nread;
    }
    return n-nleft;
}


ssize_t Writen(int fd, const void *vptr, size_t n){
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr=(char*)vptr;
    nleft=n;
    while (nleft>0) {
        if((nwritten=write(fd,ptr,nleft))<=0){
            if(nwritten<0&&errno==EINTR) nwritten=0;
            else return -1;
        }
        nleft-=nwritten;
        ptr+=nwritten;
    }
    return n;
}


static ssize_t my_read(int fd, char *ptr){
    static int read_cnt;
    static char *read_ptr;
    static char read_buf[100];

    if(read_cnt<=0){
again:
        if((read_cnt=read(fd,read_buf,sizeof(read_buf)))<0){
            if(errno==EINTR) goto again;
            return -1;
        }else if(read_cnt==0) return 0;
        read_ptr=read_buf;
    }
    read_cnt--;
    *ptr=*read_ptr++;
    return 1;
}

//readline ---fgets
//传出参数 vptr
ssize_t Readline(int fd, void *vptr, size_t maxlen){
    ssize_t n,rc;
    char c,*ptr;
    ptr=(char*)vptr;

    for(n=1;n<maxlen;++n){
        if((rc=my_read(fd, &c))==1){
            *ptr++=c;
            if(c=='\n') break;
        }else if(rc==0){
            *ptr=0;
            return n-1;
        }else {
            return -1;
        }
    }
    *ptr=0;
    return n;
}
#endif
