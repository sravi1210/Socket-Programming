// include statements
#include "../Common_header/socket.h"
#include "command_server.c"

int Socket(int family, int type, int protocol) {
    /* This function requests a socket from the os
     * It exists the application after printing error
     * if the Os cannot provide a socket
     */
    int n;
    if ((n = socket(family, type, protocol)) < 0) {
        printf("error\n");
        exit(0);
    }
    return (n);
}

int main(int argc, char **argv) {
    // declarations
    int listenfd, connfd, servport;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    char default_dir_path[1024];
    getcwd(default_dir_path, sizeof(default_dir_path));
    time_t ticks;

    if (argc != 2) {
        printf("usage: a.out <Server Port number>\n");
        return -1;
    }

    servport = atoi(argv[1]);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));  // set all bits in serveraddr 0
    servaddr.sin_family = AF_INET;       // set server IP address type to IPv4
    servaddr.sin_addr.s_addr =
        htonl(INADDR_ANY);               // accept any incoming messages
    servaddr.sin_port = htons(servport); /* daytime server */

    // Give the socket listenfd the local address servaddr
    // (which is sz(serveraddr) bytes long).
    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

    /* Prepare to accept connections on socket listenfd.
     * LISTENQ connection requests will be queued before
     * further requests are refused.
     */
    listen(listenfd, LISTENQ);

    char temp[2] = "0";
    int pid;
    int ct = 0;

    for (;;) {
        printf("\n\nWaiting for Connection\n");
        chdir(default_dir_path);

        /*Await a connection on socket FD.
         * When a connection arrives, open a
         * new socket to communicate with it,
         */
        printf("%d\n", ct);
        int t = waitpid(-1, NULL, WNOHANG);
        if (t > 0) ct--;
        printf("%d\n", t);
        if (ct == ACCEPTQ) {
            printf("WAITING\n");
            connfd = accept(listenfd, (SA *)NULL, NULL);
            strcpy(temp, "9");
            send_cmd(connfd, temp, sizeof(temp));
            close(connfd);
            wait(NULL);
            ct--;
        }
        connfd = accept(listenfd, (SA *)NULL, NULL);

        printf("Connection Successful\n\n");
        ct++;
        strcpy(temp, "8");
        send_cmd(connfd, temp, sizeof(temp));

        pid = fork();
        if (pid < 0) {
            printf("ERROR in new process creation");
        } else if (pid == 0) {
            // child process
            close(listenfd);
            while (1) {
                char temp[2] = "0";
                recv_cmd(connfd, temp, sizeof(temp));
                printf("Option received\n");
                printf("Option chosen : %s\n", temp);

                send_cmd(connfd, temp, sizeof(temp));
                printf("ACK sent\n");

                switch (temp[0]) {
                    case '1':
                        put(connfd);
                        break;
                    case '2':
                        get(connfd);
                        break;
                    case '3':
                        mput(connfd);
                        break;
                    case '4':
                        mget(connfd);
                        break;
                    case '5':
                        listDirectory(connfd);
                        break;
                    case '6':
                        changedir(connfd);
                        break;
                    case '7':
                        goto out;
                        break;
                    default:
                        printf("Command Not recognized\n");
                }
            }
        out:
            close(connfd);
            return 0;
        } else {
            // parent process
            close(connfd);
        }
    }
}
