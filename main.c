//
// Created by zhonghua dong on 2022/8/21.
//
#include "time.h"
#include "ae.h"      /* Event driven programming library */
#include "anet.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

aeEventLoop *eventLoop = NULL;

//存放错误信息的字符串
char g_err_string[1024];
#define PORT 9999
#define MAX_LEN 1024

//停止事件循环
void StopServer() {
    printf("stop server...");
    aeStop(eventLoop);
}

void ClientClose(aeEventLoop *el, int fd, int err) {
    //如果err为0，则说明是正常退出，否则就是异常退出
    if (0 == err)
        printf("Client quit: %d\n", fd);
    else if (-1 == err)
        fprintf(stderr, "Client Error: %s\n", strerror(errno));

    //删除结点，关闭文件
    aeDeleteFileEvent(el, fd, AE_READABLE);
    close(fd);
}

//有数据传过来了，读取数据
void ReadFromClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    char buffer[MAX_LEN] = {0};
    int res;
    res = read(fd, buffer, MAX_LEN);
    if (res <= 0) {
        ClientClose(el, fd, res);
    } else {
        res = write(fd, buffer, MAX_LEN);
        if (-1 == res)
            ClientClose(el, fd, res);
    }
}

//接受新连接
void AcceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    int cfd, cport;
    char ip_addr[128] = {0};
    cfd = anetTcpAccept(g_err_string, fd, ip_addr, sizeof(ip_addr), &cport);
    printf("Connected from %s:%d\n", ip_addr, cport);

    if (aeCreateFileEvent(el, cfd, AE_READABLE,
                          ReadFromClient, NULL) == AE_ERR) {
        fprintf(stderr, "client connect fail: %d\n", fd);
        close(fd);
    }
}


int main(int argc, char **argv) {
    printf("start server...\n");
// 程序停止之后会去调用这个回调函数
    signal(SIGINT, StopServer);


//    初始化网络事件循环
    eventLoop = aeCreateEventLoop(10);

    int fd = anetTcpServer(g_err_string, PORT, NULL, 100);
    if (ANET_ERR == fd)
        fprintf(stderr, "Open port %d error: %s\n", PORT, g_err_string);
    if (aeCreateFileEvent(eventLoop, fd, AE_READABLE, AcceptTcpHandler, NULL) == AE_ERR) {
        fprintf(stderr, "Unrecoverable error creating server.ipfd file event.\n");
    }

//    aeCreateTimeEvent(eventLoop, )

    aeMain(eventLoop);

    return 0;
}