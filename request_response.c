//
// Created by zhonghua dong on 2022/9/4.
//

/* request.c
*/
#include "request_response.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parse_request(
        struct httpRequest * request,
        char * http_data) {
    char * start = http_data;
    // 解析第一行
    char * method = start;
    char * url = 0;
    char * version = 0;
    for(;*start && *start != '\r'; start++) {
        //  method url version 是由 空格 分割的
        if(*start == ' ') {
            if(url == 0) {
                url = start + 1;
            } else {
                version = start + 1;
            }
            *start = '\0';
        }
    }
    *start = '\0';  // \r -> \0
    start++;   // skip \n
    request->method = method;
    request->url = url;
    request->version = version;

    /*  打印 request 信息 */
    printf("---------------------------\n");
    printf("method is: %s \n", request->method);
    printf("url is: %s \n", request->url);
    printf("http version is: %s \n", request->version);
//    printf("the headers are :\n");
//    mapPrint(request->headers);
    printf("body is %s \n", request->body);
    printf("---------------------------\n");
}

