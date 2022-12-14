//
// Created by zhonghua dong on 2022/8/27.
//
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAXSIZE     16384
#define SERV_PORT   8888
#define FDSIZE      1024
#define EPOLLEVENTS 20

struct global_args {
    int ret;        /* -p option to print what the server returns */
    int time;       /* -t option to tell how much time we will run */
    char *ip;       /* -a option to tell which server ip to connect */
    int len;        /* -l option to tell how many bytes we will send */
};

struct statistics {
    clock_t start_time;
    clock_t total_delay;
    int query_num;
    int byte_num;
};

static void handle_connection(int sock_fd, struct global_args *args,
                              struct statistics *stat);

static void handle_events(int epoll_fd, struct epoll_event *events,
                          int num, int sock_fd, char *buf, int len,
                          struct statistics *stat, int echo);

static void do_read(int epoll_fd, int sock_fd, char *buf,
                    int len, struct statistics *stat, int do_echo);

static void do_write(int epoll_fd, int sock_fd, char *buf,
                     int len, struct statistics *stat);

static void add_event(int epoll_fd, int fd, int state);

static void delete_event(int epoll_fd, int fd, int state);

static void modify_event(int epoll_fd, int fd, int state);

static void display_usage();

static void parse_args(int argc, char *argv[], struct global_args *args);

static const char *optString = "pt:a:l:h";
static int time_flag = 0;

void set_time_flag() {
    time_flag = 1;
}

int main(int argc, char *argv[]) {
    struct global_args args;
    struct sockaddr_in sockaddrIn;
    struct statistics stat;
    int sock_fd;

    args.ret = 0;
    args.time = 10;
    args.ip = "127.0.0.1";
    args.len = 10;

    memset(&stat, 0, sizeof(struct statistics));

    parse_args(argc, argv, &args);
    signal(SIGALRM, set_time_flag);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&sockaddrIn, sizeof(sockaddrIn));
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, args.ip, &sockaddrIn.sin_addr);
    connect(sock_fd, (struct sockaddr *) &sockaddrIn, sizeof(sockaddrIn));

    alarm(args.time);
    handle_connection(sock_fd, &args, &stat);

    close(sock_fd);
    return 0;
}

static void parse_args(int argc, char *argv[], struct global_args *args) {
    int opt;
    opt = getopt(argc, argv, optString);
    while (opt != -1) {
        switch (opt) {
            case 'p':
                args->ret = 1; /* true */
                break;
            case 't':
                args->time = atoi(optarg);
                break;
            case 'a':
                args->ip = optarg;
                break;
            case 'l':
                args->len = atoi(optarg);
                break;
            case 'h':   /* fall-through is intentional */
            default:
                display_usage();
                break;
        }
        opt = getopt(argc, argv, optString);
    }
}

static void display_usage() {
    printf("\t-p Print what the server returns.\n");
    printf("\t-t Specify how much time to run, default 10s\n");
    printf("\t-a Specify server IP addr, default 127.0.0.1\n");
    printf("\t-l Specify length to send in each query, default 10B\n");
    printf("\t-h Print this information\n");
    exit(0);
}

static void handle_connection(int sock_fd, struct global_args *args,
                              struct statistics *stat) {
    int epoll_fd;
    int ready_cnt;
    unsigned long byte_cnt;
    int total_delay;
    struct epoll_event events[EPOLLEVENTS];
    char *buf = (char *) malloc(args->len);
    memset(buf, 'a', args->len);

    epoll_fd = epoll_create(FDSIZE);
    add_event(epoll_fd, sock_fd, EPOLLOUT);
    while (time_flag == 0) {
        ready_cnt = epoll_wait(epoll_fd, events, EPOLLEVENTS, -1);
        handle_events(epoll_fd, events, ready_cnt, sock_fd, buf,
                      args->len, stat, args->ret);
    }

    byte_cnt = (unsigned long) stat->query_num * args->len;
    total_delay = (1000000 * stat->total_delay) / CLOCKS_PER_SEC;
    printf("QPS = %d\n", stat->query_num / args->time);
    printf("BandWidth(Byte/sec) = %lu\n", byte_cnt / args->time);
    printf("AvgDelay(ms) = %d\n", total_delay / stat->query_num);

    free(buf);
    close(epoll_fd);
}

static void
handle_events(int epoll_fd, struct epoll_event *events, int num, int sock_fd,
              char *buf, int len, struct statistics *stat, int echo) {
    int i;
    int fd;
    for (i = 0; i < num; i++) {
        fd = events[i].data.fd;
        if (events[i].events & EPOLLIN)
            do_read(epoll_fd, sock_fd, buf, len, stat, echo);
        else if (events[i].events & EPOLLOUT)
            do_write(epoll_fd, sock_fd, buf, len, stat);
    }
}

static void do_read(int epoll_fd, int sock_fd, char *buf,
                    int len, struct statistics *stat, int do_echo) {
    int nread;
    nread = read(sock_fd, buf, len);
    stat->total_delay += clock() - stat->start_time;

    if (nread == -1) {
        perror("Read error:");
        delete_event(epoll_fd, sock_fd, EPOLLIN);
        close(sock_fd);
    } else if (nread == 0) {
        fprintf(stderr, "Server closed.\n");
        delete_event(epoll_fd, sock_fd, EPOLLIN);
        close(sock_fd);
    } else {
        if (do_echo)
            printf("Server return : %s\n", buf);

        modify_event(epoll_fd, sock_fd, EPOLLOUT);
    }
}

static void do_write(int epoll_fd, int sock_fd, char *buf,
                     int len, struct statistics *stat) {
    int nwrite;
    stat->start_time = clock();
    nwrite = write(sock_fd, buf, len);
    if (nwrite == -1) {
        perror("Write error:");
        delete_event(epoll_fd, sock_fd, EPOLLOUT);
        close(sock_fd);
    } else if (nwrite == 0) {
        fprintf(stderr, "Server closed.\n");
        delete_event(epoll_fd, sock_fd, EPOLLOUT);
        close(sock_fd);
    } else {
        stat->query_num++;
        modify_event(epoll_fd, sock_fd, EPOLLIN);
    }
}

static void add_event(int epoll_fd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        printf("Add event failed!\n");
    }
}

static void delete_event(int epoll_fd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev) < 0) {
        printf("Delete event failed!\n");
    }
}

static void modify_event(int epoll_fd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        printf("Modify event failed!\n");
    }
}         