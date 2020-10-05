// include statements
#include "../Common_header/socket.h"
#include "command_client.c"

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
    int sockfd, n, servport;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    // check if server IP addr given , application port already known
    if (argc != 3) {
        printf("usage: a.out <Server IP Address> <Server Port number>\n");
        return -1;
    }

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));  // set all bits in serveraddr 0
    servaddr.sin_family = AF_INET;       // set server IP address type to IPv4

    /* The htons() function makes sure that numbers are stored in memory in
     * network byte order, which is with the most significant byte first. It
     * will therefore swap the bytes making up the number so that in memory the
     * bytes will be stored in the order
     */
    servport = atoi(argv[2]);
    servaddr.sin_port = htons(servport); /* daytime server */

    // Convert and store IP address currently in the presenatation format to
    // binary format
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s", argv[1]);
        return -1;
    }

    printf("Attempting to connect\n");

    // Open a connection on socket sockfd to peer at serveraddr
    // (which is sz(serveraddr) bytes long)
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error");
        return -1;
    }

    char temp[2];
    recv_cmd(sockfd, temp, sizeof(temp));
    if (strcmp(temp, "8") == 0) {
        printf("Connection Successful\n");
    } else {
        printf("Connection Unsuccessful\n");
        return 0;
    }

    // Dclarations : buffer and options
    char buff[1024];
    int option;

    while (1) {
        // Print options
        printf(
            "\nEnter Key For The Coomand\n 1. Put \n 2. Get \n 3. Mput \n 4. "
            "Mget \n 5. List Directory Contents \n 6. Change Directory \n 7. "
            "Close Connection\n \n");

        // Read input option
        scanf("%d", &option);

        switch (option) {
            case 1:
                // upload file to the server
                printf("Enter Filename : ");
                scanf("%s", buff);
                printf("%s\n", buff);
                if (check_file(buff)) {
                    putfile(sockfd, buff, sizeof(buff));
                } else {
                    printf("The file %s does not exist\n", buff);
                }
                break;
            case 2:
                // download file from the server
                printf("Enter Filename : ");
                scanf("%s", buff);
                getfile(sockfd, buff, sizeof(buff));
                break;
            case 3:
                // upload all files of a given extension to the server
                printf("Enter File extension : ");
                scanf("%s", buff);
                mputfile(sockfd, buff);
                break;
            case 4:
                // download all files of a given extension from the server
                printf("Enter File extension : ");
                scanf("%s", buff);
                mgetfile(sockfd, buff);
                break;
            case 5:
                // list directory contents
                listDirectory(sockfd);
                break;
            case 6:
                // change directory
                printf("Enter directory name or .. to go back : ");
                scanf("\n%s", buff);
                printf("Input\n");
                changedir(sockfd, buff);
                break;
            case 7:
                // close application
                closeConnection(sockfd);
                close(sockfd);
                goto out;
                break;
            default:
                // Undefined option specified
                printf("Command not recognized\n");
        }
    }
out:
    return 0;
}
