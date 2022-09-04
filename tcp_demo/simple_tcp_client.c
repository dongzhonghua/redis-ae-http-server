//
// Created by zhonghua dong on 2022/8/23.
//
#include "stdio.h"
#include "string.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    setbuf(stdout, NULL);
    printf("This is client\n");
    // socket
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == -1) {
        printf("Error: socket\n");
        return 0;
    }
    // connect
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(client, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        printf("Error: connect\n");
        return 0;
    }
    printf("...connect\n");
    char data[255];
    char buf[255];
    while (1) {
        scanf("%s", data);
        printf("====%s\n", data);
        send(client, data, strlen(data), 0);
        if (strcmp(data, "exit") == 0) {
            printf("...disconnect\n");
            break;
        }
        memset(buf, 0, sizeof(buf));
        int len = recv(client, buf, sizeof(buf), 0);
        buf[len] = '\0';
        printf("%s\n", buf);
    }
    close(client);
    return 0;
}
