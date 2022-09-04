//
// Created by zhonghua dong on 2022/8/28.
//

#ifndef REDIS_AE_DISPATCHER_SERVLET_H
#define REDIS_AE_DISPATCHER_SERVLET_H

#include <time.h>
#include "ae.h"
#include "request_response.h"
#include "dict.h"
#include "controller.h"

struct controller {
    char *url;
    char *method;
    // 实现函数
    serveRequest *func;
};

struct controller controllerTable[] = {
        {"/",      GET,  get_root},
        {"/hello", GET,  getHello},
        {"/hello", POST, postHello}
};

void populateCommandTable(void);


dictType controllerDictType = {
        NULL,           /* hash function */
        NULL,                      /* key dup */
        NULL,                      /* val dup */
        NULL,     /* key compare */
        NULL,         /* key destructor */
        NULL                       /* val destructor */
};

dict *controllerDict;

struct client *handleNewClient(aeEventLoop *el, int fd);

void AcceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask);

void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask);

void writeDataToClient(aeEventLoop *el, int fd, void *privdata, int mask);

void ClientClose(aeEventLoop *el, int fd, size_t err);


#define MAX_READ_BUFFER 1024
typedef struct client {
    int fd;
    char name; // 客户端的名字
    char *read_buf[MAX_READ_BUFFER]; // 查询缓冲区
    httpRequest *httpRequest;
    httpResponse *httpResponse;
    time_t ctime; // 创建客户端的时间
} client;


#endif //REDIS_AE_DISPATCHER_SERVLET_H
