#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
#include <pthread.h>
#include "../wrap.h"


#define SERV_IP      "127.0.0.1"
#define SERV_PORT     6666
#define BACK_LOG      66
#define MAX_LINE      80
#define INET_ADDR_LEN 120

typedef struct sockaddr SA;

typedef struct sockinfo {
    struct sockaddr_in cli_addr;
    int conn_fd;
} SockInfo;

void *DoThread(void *ptr) {
    pthread_detach(pthread_self());

    SockInfo *s_info = (SockInfo *)ptr;

    char buf[MAX_LINE];
    memset(buf, 0, MAX_LINE);

    while (1) {
        int nread = Read(s_info->conn_fd, buf, MAX_LINE);
        if (nread == 0) {
            printf("the other side has benn closed\n");
            break;
        }

        char str[INET_ADDR_LEN];
        printf("recvived from %s at prot %d\n",
                inet_ntop(AF_INET, &s_info->cli_addr.sin_addr, str,sizeof(str)),
                ntohs(s_info->cli_addr.sin_port));

        int i;
        for (i = 0; i < nread; ++i) buf[i] = toupper(buf[i]);
        Write(s_info->conn_fd, buf, nread);
    }

    Close(s_info->conn_fd);

    return ptr;
}

int main() {

    int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(SERV_PORT);

    Bind(listen_fd, (SA *)&serv_addr, sizeof(serv_addr));

    Listen(listen_fd, BACK_LOG);

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_addr_len = sizeof(cli_addr);

        int conn_fd = Accept(listen_fd, (SA *)&cli_addr, &cli_addr_len);

        pthread_t tid;
        SockInfo sock_info = { cli_addr, conn_fd };
        tid = pthread_create(&tid, NULL, DoThread, (void *)&sock_info);
        if (tid != 0)  {
            perror("thread create error");
            continue;
        }
    }

    return 0;
}
