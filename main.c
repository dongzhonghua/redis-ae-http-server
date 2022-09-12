//
// Created by zhonghua dong on 2022/8/21.
//
#include "time.h"
#include "ae.h"      /* Event driven programming library */
#include "anet.h"
#include "dispatcher_servlet.h"
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

//停止事件循环
void StopServer() {
    printf("stop server...");
    aeStop(eventLoop);
}

int timeEventDemo(aeEventLoop *loop, long long id, void *clientData) {
    static int i = 0;
    printf("timeEventDemo: %d\n", i++);
    //30秒后再次执行该函数
    return 30000;
}

int main(int argc, char **argv) {
    printf("start server...\n");
// 程序停止之后会去调用这个回调函数
    signal(SIGINT, StopServer);

    populateCommandTable();
//    初始化网络事件循环
    eventLoop = aeCreateEventLoop(10);

    int fd = anetTcpServer(g_err_string, PORT, NULL, 100);
    if (ANET_ERR == fd)
        fprintf(stderr, "Open port %d error: %s\n", PORT, g_err_string);
    if (aeCreateFileEvent(eventLoop, fd, AE_READABLE, AcceptTcpHandler, NULL) == AE_ERR) {
        fprintf(stderr, "Unrecoverable error creating server.ipfd file event.\n");
    }

//    aeCreateTimeEvent(eventLoop, 1, timeEventDemo, NULL, NULL);

    aeMain(eventLoop);

    return 0;
}