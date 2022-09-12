//
// Created by zhonghua dong on 2022/9/4.
//

#ifndef REDIS_AE_REQUEST_RESPONSE_H
#define REDIS_AE_REQUEST_RESPONSE_H
/* request_response.h
*/

#include "dict.h"

#ifndef __REQUEST_RESPONSE__
#define __REQUEST_RESPONSE__

#define GET "GET"
#define POST "POST"

typedef struct httpRequest {
    char *method;
    char *url;
    char *version;
    dict *headers;
    char *body;
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


void parse_request(
        struct httpRequest *request,
        char *http_data);

#endif
#endif //REDIS_AE_REQUEST_RESPONSE_H
