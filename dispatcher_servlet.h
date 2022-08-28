//
// Created by zhonghua dong on 2022/8/28.
//

#ifndef REDIS_AE_DISPATCHER_SERVLET_H
#define REDIS_AE_DISPATCHER_SERVLET_H

#include <time.h>
#include "ae.h"

struct client *handleNewClient(aeEventLoop *el, int fd);

void AcceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask);

void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask);

void writeDataToClient(aeEventLoop *el, int fd, void *privdata, int mask);

void ClientClose(aeEventLoop *el, int fd, int err);


typedef struct httpRequest {
    char *method;
    char *url;
    char *version;
    char *host;
    char *connection;
    char *userAgent;
} httpRequest;

typedef struct httpResponse {
    char *version;
    int code;
    char *date;
    char *server;
    char *contentType;
    int contentLength;
    char *content;
} httpResponse;

typedef struct client {
    int fd;
    char name; // 客户端的名字
    char *read_buf[1024]; // 查询缓冲区
    httpRequest httpRequest;
    httpResponse httpResponse;
    time_t ctime; // 创建客户端的时间
} client;


#endif //REDIS_AE_DISPATCHER_SERVLET_H
