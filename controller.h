//
// Created by zhonghua dong on 2022/9/4.
//
#include "request_response.h"
#include "dict.h"
#ifndef REDIS_AE_CONTROLLER_H
#define REDIS_AE_CONTROLLER_H

typedef void HandleRequesFunc(httpRequest *request, httpResponse *response);

void page_not_found(httpRequest *request, httpResponse *response);

void get_root(httpRequest *request, httpResponse *response);

void get_hello(httpRequest *request, httpResponse *response);

void post_hello(httpRequest *request, httpResponse *response);

#endif //REDIS_AE_CONTROLLER_H
