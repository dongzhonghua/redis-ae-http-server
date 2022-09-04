//
// Created by zhonghua dong on 2022/9/4.
//
#include "request_response.h"

#ifndef REDIS_AE_CONTROLLER_H
#define REDIS_AE_CONTROLLER_H

typedef void serveRequest(httpRequest *request, httpResponse *response);

void get_root(httpRequest *request, httpResponse *response);

void getHello(httpRequest *request, httpResponse *response);

void postHello(httpRequest *request, httpResponse *response);

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

#endif //REDIS_AE_CONTROLLER_H
