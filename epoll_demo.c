
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXFDS 1024
#define EVENTS_NUM 100
#define PORT 8881

int epfd;

int main(int args) {
    setbuf(stdout, NULL);
    printf("This is server\n");
    int listen_fd, nfds, client_fd;
    int on = 1;
    char *buffer[512];
    struct sockaddr_in server_addr;
    struct epoll_event ev, events[EVENTS_NUM];
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        printf("创建套接字出错了！");
        return -1;
    }
// ???
//    struct sigaction sig;
//    sigemptyset(&sig.sa_mask);
//    sig_handler = SIG_IGN;
//    sigaction(SIGPIPE, &N > sig, NULL);

//    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); // ???

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        printf("套接字不能绑定到服务器上！");
        return -1;
    }


    if (-1 == listen(listen_fd, 32)) {
        printf("监听套接字的时候出错了");
        return -1;
    }
    printf("...listen\n");

    epfd = epoll_create(MAXFDS);

    ev.data.fd = listen_fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    char clientIP[INET_ADDRSTRLEN] = "";
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    while (1) {
        nfds = epoll_wait(epfd, events, MAXFDS, 500);
        for (int i = 0; i < nfds; ++i) {
            if (listen_fd == events[i].data.fd) {
                memset(&clientAddr, 0, sizeof(clientAddr));
                client_fd = accept(listen_fd, (struct sockaddr *) &clientAddr, &clientAddrLen);

                inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                printf("...connect  %s %hu\n", clientIP, ntohs(clientAddr.sin_port));

                if (-1 == client_fd) {
                    printf("服务器接受连接出错\n");
                    break;
                }

//                setNonBlock(client_fd); // ???作用
                ev.data.fd = client_fd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
            } else if (events[i].data.fd & EPOLLIN) {
                bzero(&buffer, sizeof(buffer));
                printf("服务器端要读取客户端发过来的消息\n");
                int ret = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                if (ret < 0) {
                    printf("服务器收到的消息出错了\n");
                    return -1;
                }
                buffer[ret] = '\0';
                printf("接收到的消息为：%s\n", (char *) buffer);
                ev.data.fd = events[i].data.fd;
                ev.events = EPOLLOUT | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
            } else if (events[i].data.fd & EPOLLOUT) {
                printf("服务器发送消息给客户端");
                bzero(&buffer, sizeof(buffer));
                bcopy("The Author@: magicminglee@Hotmail.com", buffer, sizeof("The Author@: magicminglee@Hotmail.com"));
                int ret = send(events[i].data.fd, buffer, strlen((const char *) buffer), 0);
                if (ret < 0) {
                    printf("服务器发送消息给客户端的时候出错啦");
                    return -1;
                }
                ev.data.fd = events[i].data.fd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_MOD, ev.data.fd, &ev);
            }
        }
    }
    close(listen_fd);
    return 0;
}