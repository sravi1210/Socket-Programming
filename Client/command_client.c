#include "../Common_header/common_function.c"
#include "../Common_header/socket.h"

int getfile(int fd, char *ptr, int size) {
    /* get a file from the server
     * INPUTS :=
     * fd : socket
     * ptr : buffer with filename
     * size : len of filename
     */
    char temp[2] = "2";
    char buff[1024];
    strcpy(buff, ptr);

    /* Check if the desired file exists
     * If exists then ask the user if he wants to overwrite
     * the previous file
     */
    if (check_file(buff)) {
        char option;

    here:
        printf("Do You Wish To Overwrite %s Y/N :", buff);
        scanf("\n%c", &option);

        if (option == 'N' || option == 'n') {
            return 0;
        } else if (option != 'Y' && option != 'y') {
            printf("Option not recognized %c\n", option);
            goto here;
        }
    }

    send_cmd(fd, temp, sizeof(temp));
    printf("Sending Option...\n");

    recv_cmd(fd, temp, sizeof(temp));
    printf("Recieve ACK from Server %s\n", temp);

    send_cmd(fd, buff, sizeof(buff));
    printf("Sending File name : %s\n", buff);

    /* Receive file from the server
     * use ack and messages to communicate with the server
     */
    if (recv_confirm(fd)) {
        printf("File found on server. Confirm Recieved\n");

        send_cmd(fd, temp, sizeof(temp));
        printf("ACK sent\n");
        if (!recv_confirm(fd)) {
            printf("File could not be opened\n");
            return 0;
        }
        if (!recv_confirm(fd)) {
            printf("File size too large, try again later\n");
            return 0;
        }
        long int file_size = recv_file_size(fd);

        send_cmd(fd, temp, sizeof(temp));
        printf("ACK sent\n");

        recv_file(fd, buff, file_size);

        send_cmd(fd, temp, sizeof(temp));
        printf("ACK sent\n");

        recv_cmd(fd, temp, sizeof(temp));
        printf("ACK recieved\n");
    } else {
        printf("The requested file does not exist\n");
    }
    return 0;
}

int putfile(int fd, char *ptr, int size) {
    // upload file to the server
    char temp[2] = "1";
    char buff[1024];
    strcpy(buff, ptr);

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));

    send_cmd(fd, buff, sizeof(buff));
    printf("This is ..................%s %ld\n", buff, sizeof(buff));

    /* Check if the desired file exists
     * If exists then ask the user if he wants to overwrite
     * the previous file
     * When the file has no counterpart present on the server then upload
     * the file on the server
     */

    if (recv_confirm(fd)) {
        char option;
    recv_confirmation:
        printf(
            "The file already exists on remote host. Do you wish to "
            "overwrite(y/n) "
            ": ");
        scanf("\n%s", &option);

        if (option == 'N' || option == 'n') {
            send_confirm(fd, false);
            recv_cmd(fd, temp, sizeof(temp));
            return 0;
        } else if (option == 'Y' || option == 'y') {
            send_confirm(fd, true);
            recv_cmd(fd, temp, sizeof(temp));
        } else if (option != 'Y' && option != 'y') {
            printf("Option not recognized %c\n", option);
            goto recv_confirmation;
        }
    } else {
        send_confirm(fd, true);
        recv_cmd(fd, temp, sizeof(temp));
    }

    if (send_file(fd, buff) == 0) {
        recv_cmd(fd, temp, sizeof(temp));
    }

    return 0;
}

int mgetfile(int fd, char *ptr) {
    // get all files one by one having a given extension
    char temp[2] = "4";
    char buff[1024];
    strcpy(buff, ptr);

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));
    send_cmd(fd, buff, sizeof(buff));
    while (recv_confirm(fd)) {
        send_cmd(fd, temp, sizeof(temp));
        recv_one_file(fd);
    }

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));
}

int mputfile(int fd, char *ptr) {
    // put all files one by one having a given extension
    char temp[2] = "3";
    char buff[1024];
    char dir_path[1024];
    strcpy(buff, ptr);

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));

    /* For each file in the given directory having the given
     * extension upload that file to the server
     */

    DIR *d = opendir(getcwd(dir_path, sizeof(dir_path)));
    struct dirent *dir;
    if (d) {
        while ((dir = readdir(d))) {
            if (dir->d_type == DT_REG) {
                char extbuf[1024];
                get_filename_ext(dir->d_name, extbuf);
                if (strcmp(extbuf, buff) == 0) {
                    send_confirm(fd, true);
                    recv_cmd(fd, temp, sizeof(temp));
                    put_one_file(fd, dir->d_name);
                }
            }
        }
    }
    send_confirm(fd, false);
    recv_cmd(fd, temp, sizeof(temp));
    return 0;
}

int closeConnection(int fd) {
    // instructs the server to close the connection
    char temp[2] = "7";
    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));
    return 0;
}

int listDirectory(int fd) {
    // retrieve directory info from servers
    char temp[2] = "5";
    char buff[1024];

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));
    int count = 1;
    while (recv_confirm(fd)) {
        recv_cmd(fd, buff, sizeof(buff));
        printf("%d .    %s\n", count, buff);
        count++;
    }
    return 0;
}

int changedir(int fd, char *ptr) {
    // change the current directory at the server
    char temp[2] = "6";
    char buff[1024];
    strcpy(buff, ptr);

    send_cmd(fd, temp, sizeof(temp));
    recv_cmd(fd, temp, sizeof(temp));

    send_cmd(fd, buff, sizeof(buff));
    recv_cmd(fd, buff, sizeof(buff));

    printf("Message : %s\n", buff);

    recv_cmd(fd, buff, sizeof(buff));
    printf("Current directory : %s\n", buff);

    return 0;
}
