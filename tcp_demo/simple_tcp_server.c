//
// Created by zhonghua dong on 2022/8/23.
//

#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    setbuf(stdout, NULL);
    printf("This is server\n");
    // socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        printf("Error: socket\n");
        return 0;
    }
    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("Error: bind\n");
        return 0;
    }
    // listen
    if (listen(listenfd, 5) == -1) {
        printf("Error: listen\n");
        return 0;
    }
    int conn;
    char clientIP[INET_ADDRSTRLEN] = "";
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    while (1) {
        printf("...listening\n");
        // accept
        conn = accept(listenfd, (struct sockaddr *) &clientAddr, &clientAddrLen);
        if (conn < 0) {
            printf("Error: accept\n");
            continue;
        }
        // 将IPv4或IPv6 Internet网络地址转换为 Internet标准格式的字符串。
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        printf("...connect  %s %hu\n", clientIP, ntohs(clientAddr.sin_port));
        char buf[255];
        while (1) {
            memset(buf, 0, sizeof(buf));
            // read
            int len = recv(conn, buf, sizeof(buf), 0);
            buf[len] = '\0';
            if (len == 0 || strcmp(buf, "exit") == 0) {
                printf("...disconnect  %s %hu\n", clientIP, ntohs(clientAddr.sin_port));
                break;
            }
            printf("%s\n", buf);
            // write
            send(conn, buf, len, 0);
        }
        close(conn);
    }
    close(listenfd);
    return 0;
}