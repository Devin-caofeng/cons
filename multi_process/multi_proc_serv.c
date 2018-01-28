#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include "../wrap.h"

#define MAX_LINE      80
#define SERV_PORT     6666
#define BACK_LOG      66
#define INET_ADDR_LEN 120

typedef struct sockaddr SA;

void DoSigChild(int signum) {
    while (waitpid(0, NULL, WNOHANG) > 0) {
        ;
    }
}

int main() {

    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    new_action.sa_handler = DoSigChild;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGCHLD, &new_action, NULL);

    int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    Bind(listen_fd, (SA *)&serv_addr, sizeof(serv_addr));

    Listen(listen_fd, BACK_LOG);

    printf("accepting connections...\n");
    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_addr_len = sizeof(cli_addr);
        int cli_fd = Accept(listen_fd, (SA *)&cli_addr, &cli_addr_len);

        pid_t pid = fork();
        if (pid == 0) {
            Close(listen_fd);
            while (1) {
                char buf[MAX_LINE];
                int n = Read(cli_fd, buf, MAX_LINE);
                if (n == 0) {
                    printf("the other side has been closed\n");
                    break;
                }

                char str[INET_ADDR_LEN];
                printf("recvied from %s at port %d\n",
                        inet_ntop(AF_INET, &cli_addr.sin_addr,
                                  str, sizeof(str)),
                        ntohs(cli_addr.sin_port));

                int i;
                for (i = 0; i < n; ++i) buf[i] = toupper(buf[i]);

                Write(cli_fd, buf, n);
            }
            Close(cli_fd);

            return 0;
        }
        else if (pid > 0) {
            Close(cli_fd);
        }
        else {
            PerrAndExit("fork error");
        }
    }

    Close(listen_fd);

    return 0;
}

