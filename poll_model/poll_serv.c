#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <ctype.h>
#include "../wrap.h"

#define MAX_LINE  80
#define SERV_IP   "192.168.42.134"
#define SERV_PORT 9999
#define OPEN_MAX  1024
#define BACK_LOG  20

typedef struct sockaddr SA;

int main() {

    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (SA *)&serv_addr, sizeof(serv_addr));

    Listen(listenfd, BACK_LOG);

    struct pollfd client[OPEN_MAX];
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;  // listenfd listen normal read event

    for (int i = 1; i < OPEN_MAX; ++i) {
        client[i].fd = -1;
    }
    int maxi = -1;

    while (1) {
        int nready = poll(client, maxi + 1, -1);  // block

        if (client[0].revents & POLLRDNORM) {  // new client connection
            struct sockaddr_in cli_addr;
            socklen_t cli_addr_len = sizeof(cli_addr);
            int connfd = Accept(listenfd, (SA *)&cli_addr, &cli_addr_len);

            char buf[MAX_LINE];
            printf("received from %s at port %d\n",
                    inet_ntop(AF_INET, &cli_addr.sin_addr, buf, sizeof(buf)),
                    ntohs(cli_addr.sin_port));

            int i;
            for (i = 1; i < OPEN_MAX; ++i) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    client[i].events = POLLRDNORM;
                    break;
                }
            }
            if (i == OPEN_MAX) PerrAndExit("too many clients");

            if (i > maxi) maxi = i;
            if (--nready == 0) continue;  // no ready event, continue
        }

        for (int i = 1; i <= maxi; ++i) {
            int sockfd = client[i].fd;
            if (sockfd < 0) continue;

            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                char buf[MAX_LINE];
                ssize_t n = Read(sockfd, buf, MAX_LINE);
                if (n < 0) {
                    if (errno == ECONNRESET) {
                        // connection reset by client
                        printf("client[%d] aborted connection\n", i);
                        Close(sockfd);
                        client[i].fd = -1;
                    }
                    else {
                        PerrAndExit("read error");
                    }
                }
                else if (n == 0) {
                    printf("client[%d] closed connection\n", i);
                    Close(sockfd);
                    client[i].fd = -1;
                }
                else {
                    for (int j = 0; j < n; ++j) {
                        buf[j] = toupper(buf[j]);
                    }
                    Write(sockfd, buf, n);
                }

                if (--nready == 0) break;
            }
        }
    }

    Close(listenfd);

    return 0;
}
