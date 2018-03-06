#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>
#include "../wrap.h"

#define MAX_LINE  80
#define SERV_IP   "192.168.42.134"
#define SERV_PORT 9999
#define OPEN_MAX  1024
#define BACK_LOG  20
#define EPOLL_SIZE 666

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

    int client[EPOLL_SIZE];
    for (int i = 0; i < EPOLL_SIZE; ++i) {
        client[i] = -1;
    }

    int epollfd = epoll_create(EPOLL_SIZE);
    struct epoll_event evs[EPOLL_SIZE], ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    while (1) {
        int nfds = epoll_wait(epollfd, evs, EPOLL_SIZE, -1);
        if (nfds < 0) {
            PerrAndExit("epoll_wait error");
        }
        else if (nfds == 0) {
            ;
        }

        for (int i = 0; i < nfds; ++i) {
            if (evs[i].data.fd == listenfd) {
                struct sockaddr_in cli_addr;
                socklen_t cli_addr_len = sizeof(cli_addr);
                int connfd = Accept(listenfd, (SA *)&cli_addr, &cli_addr_len);

                char buf[MAX_LINE];
                printf("received from %s at port %d\n",
                        inet_ntop(AF_INET, &cli_addr.sin_addr,
                                  buf, sizeof(buf)),
                        ntohs(cli_addr.sin_port));

                int i;
                for (i = 0; i < EPOLL_SIZE; ++i) {
                    if (client[i] == -1) {
                        client[i] = connfd;
                        break;
                    }
                }
                if (i == EPOLL_SIZE) {
                    PerrAndExit("too many client");
                }

                ev.data.fd = connfd;
                ev.events = EPOLLIN;
                int res = epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
                if (res == -1) {
                    PerrAndExit("epoll_ctl error");
                }
            }  // evs[i].fd == listenfd
            else {
                int sockfd = evs[i].data.fd;
                char buf[MAX_LINE];
                int n = Read(sockfd, buf, MAX_LINE);
                if (n == 0) {
                    for (int j = 0; j < n; ++j) {
                        if (client[j] == sockfd) {
                            client[j] = -1;
                            printf("client[%d] close connection", j);
                            break;
                        }
                    }

                    int res = epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);
                    if (res == -1) {
                        PerrAndExit("epoll_ctl error");
                    }
                    Close(sockfd);
                }
                else {
                    for (int i = 0; i < n; ++i) {
                        buf[i] = toupper(buf[i]);
                    }
                    Write(sockfd, buf, n);
                }
            }
        }
    }

    Close(epollfd);
    Close(listenfd);

    return 0;
}
