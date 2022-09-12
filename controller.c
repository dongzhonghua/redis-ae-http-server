//
// Created by zhonghua dong on 2022/9/4.
//

#include <memory.h>
#include <stdio.h>
#include <malloc.h>
#include "controller.h"

void get_root(httpRequest *request, httpResponse *response) {
    char *content = "this is root";
    response->content = content;
}

void get_hello(httpRequest *request, httpResponse *response) {
    char *content = "hello word!";
    response->content = content;
}

void post_hello(httpRequest *request, httpResponse *response) {
    char *content = (char*)malloc(sizeof(char)*(sizeof(request->body) + 6));
    sprintf(content, "hello %s", request->body);
    response->content = content;
}

void page_not_found(httpRequest *request, httpResponse *response) {
    char *page_not_found_content = "<html>\n"
                                   "<head>\n"
                                   "<meta charset=\"UTF-8\">\n"
                                   "<title>Hello world</title>\n"
                                   "</head>\n"
                                   "<body>\n"
                                   "<p>page not found!</p>\n"
                                   "<p><a href=/>访问主页</a></p>\n"
                                   "</body>\n"
                                   "</html>";
    response->content = page_not_found_content;
}
