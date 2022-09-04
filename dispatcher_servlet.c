//
// Created by zhonghua dong on 2022/8/28.
//

#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "dispatcher_servlet.h"
#include "anet.h"
#include "ae.h"
#include "ae_epoll.c"

#define MAX_LEN 1024
//存放错误信息的字符串
char g_err_string[1024];

const char content[] = "HTTP/1.1 200 OK\n"
                       "Date: Sun, 28 Aug 2022 06:29:09 GMT\n"
                       "Server: dzh\n"
                       "Content-Type: text/html\n"
                       "Content-Length: 112\n"
                       "\n"
                       "<html>\n"
                       "<head>\n"
                       "<meta charset=\"UTF-8\">\n"
                       "<title>Hello world</title>\n"
                       "</head>\n"
                       "<body>\n"
                       "<p>你好！</p>\n"
                       "</body>\n"
                       "</html>";

void populateCommandTable(void) {
    int j;
    controllerDict = dictCreate(&controllerDictType, NULL);
    // 命令的数量
    int commands_num = sizeof(controllerTable) / sizeof(struct controller);

    for (j = 0; j < commands_num; j++) {
        // 指定命令
        struct controller *c = controllerTable + j;
        // 将命令关联到命令表
        dictAdd(controllerDict, strcat(c->url, c->method), c);
    }
}

void ClientClose(aeEventLoop *el, int fd, size_t err) {
    //如果err为0，则说明是正常退出，否则就是异常退出
    if (0 == err)
        printf("Client quit: %d\n", fd);
    else if (-1 == err)
        fprintf(stderr, "Client Error: %s\n", strerror(errno));

    //删除结点，关闭文件
    aeDeleteFileEvent(el, fd, AE_READABLE);
    close(fd);
}


// accept
//接受新连接
void AcceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    int cfd, cport;
    char ip_addr[128] = {0};
    cfd = anetTcpAccept(g_err_string, fd, ip_addr, sizeof(ip_addr), &cport);
    printf("Connected from %s:%d\n", ip_addr, cport);
    // 创建客户端
    if (handleNewClient(el, cfd) == NULL) {
        printf("create client error, %s, %d", strerror(errno), fd);
        close(fd);
        return;
    }
}

#define PARSE_REQUEST_INFO 0
#define PARSE_HEADER 1
#define PARSE_CONTENT 2

// read
//有数据传过来了，读取数据
void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    client *c = (client *) privdata;
    size_t res;
    char *readBuf = (char *) c->read_buf;
    memset(readBuf, 0, sizeof(*readBuf));
    res = read(fd, readBuf, MAX_LEN);
    printf("================read================\n");
    printf("%s\n", (const char *) readBuf);
    if (res <= 0) {
        ClientClose(el, fd, res);
        return;
    }
    // 1. 解析请求
    parse_request(c->httpRequest, readBuf);
    // 2. 处理请求

    // 3. 绑定写事件到事件 loop，等待发送返回信息
    if (aeCreateFileEvent(el, fd, AE_WRITABLE,
                          writeDataToClient, c) == AE_ERR) {
        close(fd);
        free(c);
    }
}


// write
void writeDataToClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    printf("================write================\n");
    size_t res = write(fd, content, sizeof(content));
    if (-1 == res)
        ClientClose(el, fd, res);
    aeDeleteFileEvent(el, fd, AE_WRITABLE);
    // 这里可能不需要在绑定读事件了，直接删除写事件就可以
//    if (aeCreateFileEvent(el, fd, AE_READABLE,
//                          writeDataToClient, privdata) == AE_ERR) {
//        close(fd);
//        free(privdata);
//    }
}

struct client *handleNewClient(aeEventLoop *el, int fd) {
    client *c = malloc(sizeof(client));
    c->httpRequest = malloc(sizeof(httpRequest));
    c->httpResponse = malloc(sizeof(httpResponse));

    c->fd = fd;

    // 非阻塞
    anetNonBlock(NULL, fd);
    // 禁用 Nagle 算法
    anetEnableTcpNoDelay(NULL, fd);
    // 设置 keep alive
    anetKeepAlive(NULL, fd, 1);
    // 绑定读事件到事件 loop （开始接收命令请求）
    if (aeCreateFileEvent(el, fd, AE_READABLE,
                          readQueryFromClient, c) == AE_ERR) {
        close(fd);
        free(c);
        return NULL;
    }
    return c;
}