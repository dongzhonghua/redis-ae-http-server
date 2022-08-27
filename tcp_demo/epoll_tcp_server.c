//
// Created by zhonghua dong on 2022/8/27.
//
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define IPADDRESS   "127.0.0.1"
#define PORT        8888
#define MAXSIZE     87380
#define LISTENQ     5
#define FDSIZE      1000
#define EPOLLEVENTS 100

static int socket_bind(const char* ip, int port);
static void do_epoll(int listen_fd);
static void handle_events(int epoll_fd, struct epoll_event *events,
                          int num, int listen_fd, char *buf);
static void handle_accept(int epoll_fd, int listen_fd);
static void do_read(int epoll_fd, int fd, char *buf);
static void do_write(int epoll_fd, int fd, char *buf);
static void add_event(int epoll_fd, int fd, int state);
static void modify_event(int epoll_fd, int fd, int state);
static void delete_event(int epoll_fd, int fd, int state);

int main(int argc,char *argv[]) {
    int  listen_fd = socket_bind(IPADDRESS, PORT);
    listen(listen_fd, LISTENQ);
    do_epoll(listen_fd);
    return 0;
}

static int socket_bind(const char* ip,int port) {
    int  listen_fd;
    struct sockaddr_in servaddr;
    listen_fd = socket(AF_INET,SOCK_STREAM,0);
    if (listen_fd == -1) {
        perror("socket error:");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if (bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind error: ");
        exit(1);
    }
    return listen_fd;
}

static void do_epoll(int listen_fd) {
    int epoll_fd;
    struct epoll_event events[EPOLLEVENTS];
    int ready_cnt;
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    epoll_fd = epoll_create(FDSIZE);
    add_event(epoll_fd, listen_fd, EPOLLIN);
    for ( ; ; ) {
        ready_cnt = epoll_wait(epoll_fd, events, EPOLLEVENTS, -1);
        handle_events(epoll_fd, events, ready_cnt, listen_fd, buf);
    }
    close(epoll_fd);
}

static void
handle_events(int epoll_fd, struct epoll_event *events, int num,
              int listen_fd, char *buf) {
    int i;
    int fd;

    for (i = 0; i < num; i++) {
        fd = events[i].data.fd;
        // If fd is a listen fd, we do accept(), otherwise it is a
        // connected fd, we should read buf if EPOLLIN occured.
        if ((fd == listen_fd) && (events[i].events & EPOLLIN))
            handle_accept(epoll_fd, listen_fd);
        else if (events[i].events & EPOLLIN)
            do_read(epoll_fd, fd, buf);
        else if (events[i].events & EPOLLOUT)
            do_write(epoll_fd, fd, buf);
    }
}
static void handle_accept(int epoll_fd, int listen_fd) {
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t  cliaddrlen;

    clifd = accept(listen_fd, (struct sockaddr*) &cliaddr, &cliaddrlen);
    if (clifd == -1)
        perror("Accpet error:");
    else {
        printf("Accept a new client: %s:%d\n",
               inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
        add_event(epoll_fd, clifd, EPOLLIN);
    }
}

static void do_read(int epoll_fd, int fd, char *buf) {
    int nread;

    nread = read(fd, buf, MAXSIZE);
    if (nread == -1) {
        perror("Read error:");
        delete_event(epoll_fd, fd, EPOLLIN);
        close(fd);
    }
    else if (nread == 0) {
        fprintf(stderr, "Client closed.\n");
        delete_event(epoll_fd, fd, EPOLLIN);
        close(fd);
    }
    else {
        //printf("Read message is : %s", buf);
        modify_event(epoll_fd, fd, EPOLLOUT);
    }
}

static void do_write(int epoll_fd, int fd, char *buf) {
    int nwrite;

    nwrite = write(fd, buf, strlen(buf));
    if (nwrite == -1) {
        perror("Write error:");
        delete_event(epoll_fd, fd, EPOLLOUT);
        close(fd);
    }
    else
        modify_event(epoll_fd, fd, EPOLLIN);

    memset(buf, 0, MAXSIZE);
}

static void add_event(int epoll_fd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        printf("Add event failed!\n");
    }
}

static void delete_event(int epoll_fd,int fd,int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev) < 0) {
        printf("Delete event failed!\n");
    }
}

static void modify_event(int epoll_fd,int fd,int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        printf("Modify event failed!\n");
    }
}