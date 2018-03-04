#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "../wrap.h"

#define MAX_LINE    80
#define LISTEN_NUM  20
#define SERVER_PORT 9999
#define SERVER_IP   "192.168.42.134"
#define INET_ADDR_STR_LEN 16

typedef struct sockaddr SA;

int main() {

    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(SERVER_PORT);

    Bind(listenfd, (SA *)&serv_addr, sizeof(serv_addr));

    Listen(listenfd, LISTEN_NUM);

    int maxfd = listenfd;
    int maxi = -1;
    int client[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; ++i) {
        client[i] = -1;
    }

    fd_set rset, all_set;
    FD_ZERO(&all_set);
    FD_SET(listenfd, &all_set);

    while (1) {
        rset = all_set;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            PerrAndExit("select error");
        }

        if (FD_ISSET(listenfd, &rset)) {  // new client connection
            struct sockaddr_in cli_addr;
            socklen_t cli_addr_len = sizeof(cli_addr);
            int connfd = Accept(listenfd, (SA *)&cli_addr, &cli_addr_len);

            char str[INET_ADDR_STR_LEN];
            printf("recvived from %s at port %d\n",
                    inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)),
                    ntohs(cli_addr.sin_port));

            int i;
            for (i = 0; i < FD_SETSIZE; ++i) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                fputs("too many clients\n", stderr);
                exit(1);
            }

            FD_SET(connfd, &all_set);
            if (connfd > maxfd) {
                maxfd = connfd;
            }
            if (i > maxi) {
                maxi = i;
            }

            if (--nready == 0) {
                continue;
            }
        }

        for (int i = 0; i <= maxi; ++i) {
            int sockfd = client[i];
            if (sockfd < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &rset)) {
                char buf[MAX_LINE];
                ssize_t n = Read(sockfd, buf, MAX_LINE);
                if (n == 0) {
                    // Server close connection when client close this connection
                    Close(sockfd);
                    // delete this sockfd from select set
                    FD_CLR(sockfd, &all_set);
                    client[i] = -1;
                }
                else {
                    for (int j = 0; j < n; ++j) {
                        buf[j] = toupper(buf[j]);
                    }
                    Write(sockfd, buf, n);
                }

                if (--nready == 0) {
                    break;
                }
            }
        }
    }

    Close(listenfd);

    return 0;
}
