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
        struct httpRequest *request,
        char *http_data) {
    char *start = http_data;
    // 解析第一行
    char *method = start;
    char *url = 0;
    char *version = 0;
    for (; *start && *start != '\r'; start++) {
        //  method url version 是由 空格 分割的
        if (*start == ' ') {
            if (url == 0) {
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



    /* 第一行解析结束 */
    /* 第二行开始为 header  解析hedaer*/
    request->headers = dictCreate(&dictTypeHeapStrings, NULL);

    start++;   // 第二行开始
    char *line = start;
    char *key;
    char *value;
    while (*line != '\r' && *line != '\0') {
        char *key;
        char *value;
        while (*(start++) != ':');
        *(start - 1) = '\0';
        key = line;
        value = start;
        while (start++, *start != '\0' && *start != '\r');
        *start++ = '\0'; // \r -> \0
        start++;   // skip \n
        line = start;
        dictAdd(request->headers, key, value);
    }
    /* 如果最后一行不是空行  说明有body数据 */
    if (*line == '\r') {
        char *len_str = dictFetchValue(request->headers, "Content-Length");
        if (len_str != NULL) {
            int len = atoi(len_str);
            // 跳过 两个 \n
            line = line + 2;
            *(line + len) = '\0';
            request->body = line;
        }
    }

    /*  打印 request 信息 */
    printf("---------------------------\n");
    printf("method is: %s \n", request->method);
    printf("url is: %s \n", request->url);
    printf("http version is: %s \n", request->version);
    printf("headers are :\n");
    dictIterator *headersIterator = dictGetIterator(request->headers);
    dictEntry *de;
    while ((de = dictNext(headersIterator)) != NULL) {
        char *k = dictGetKey(de);
        char *v = dictGetVal(de);
        printf("\theader key: %s, value: %s\n", k, v);
    }
    printf("body is %s \n", request->body);
    printf("---------------------------\n");
    printf("---------------------------\n");
}

