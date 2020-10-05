#include "../Common_header/common_function.c"
#include "../Common_header/socket.h"

int get(int connfd) {
    // send a file to the client
    char buff[1024];
    char temp[2] = "2";

    recv_cmd(connfd, buff, sizeof(buff));
    printf("Requested File : %s\n", buff);

    if (check_file(buff)) {
        send_confirm(connfd, true);
        printf("Confirmation sent\n");

        recv_cmd(connfd, temp, sizeof(temp));
        printf("ACK recieved\n");

        if (send_file(connfd, buff) < 0) {
            return 0;
        }

        recv_cmd(connfd, temp, sizeof(temp));
        printf("ACK recieved\n");

        send_cmd(connfd, temp, sizeof(temp));
        printf("ACK sent\n");
    } else {
        send_confirm(connfd, false);
        printf("Confirmation sent\n");
        printf("Requested File Not Found : %s\n", buff);
    }

    return 0;
}

int put(int fd) {
    // recieve and upload a file from the client to the server
    char buff[1024];
    char temp[2] = "1";

    recv_cmd(fd, buff, sizeof(buff));

    if (check_file(buff)) {
        send_confirm(fd, true);
        if (recv_confirm(fd)) {
            send_cmd(fd, temp, sizeof(temp));
        } else {
            send_cmd(fd, temp, sizeof(temp));
            return 0;
        }
    } else {
        send_confirm(fd, false);
        recv_confirm(fd);
        send_cmd(fd, temp, sizeof(temp));
    }
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

    recv_file(fd, buff, file_size);
    send_cmd(fd, temp, sizeof(temp));

    return 0;
}

int mput(int fd) {
    /* Recieve files of a particular file from a client
     * and uplad all of them to the server
     */
    char buff[1024];
    char temp[2] = "3";
    while (recv_confirm(fd)) {
        send_cmd(fd, temp, sizeof(temp));
        recv_put_one_file(fd);
    }
    send_cmd(fd, temp, sizeof(temp));
    return 0;
}

int mget(int fd) {
    /* Send all files of an extension to the client
     * Receive the extention from the client
     */
    char buff[1024];
    char dir_path[1024];
    char temp[2] = "4";

    recv_cmd(fd, buff, sizeof(buff));
    DIR *d;
    struct dirent *dir;
    d = opendir(getcwd(dir_path, sizeof(dir_path)));
    if (d) {
        while ((dir = readdir(d))) {
            if (dir->d_type == DT_REG) {
                char extbuf[1024];
                get_filename_ext(dir->d_name, extbuf);
                if (strcmp(extbuf, buff) == 0) {
                    send_confirm(fd, true);
                    recv_cmd(fd, temp, sizeof(temp));
                    send_one_file(fd, dir->d_name);
                }
            }
        }
    }

    send_confirm(fd, false);
    recv_cmd(fd, temp, sizeof(temp));
    send_cmd(fd, temp, sizeof(temp));

    return 0;
}

int listDirectory(int fd) {
    // Send the directory structure to the client
    char buff[1024];

    DIR *dir = opendir(getcwd(buff, sizeof(buff)));
    struct dirent *d;
    while (dir && (d = readdir(dir))) {
        send_confirm(fd, true);
        strcpy(buff, d->d_name);
        send_cmd(fd, buff, sizeof(buff));
    }

    send_confirm(fd, false);
    return 0;
}

int changedir(int fd) {
    // change the current directory to the directory specified by the client
    char buff[1024];
    char temp[2] = "6";

    recv_cmd(fd, buff, sizeof(buff));
    if (chdir(buff) == 0) {
        strcpy(buff, "Directory Changed");
    } else {
        if (errno == EACCES) {
            strcpy(buff, "Directory Access Permission Denied");
        } else {
            strcpy(buff, "Directory Does not exist");
        }
    }
    send_cmd(fd, buff, sizeof(buff));

    getcwd(buff, sizeof(buff));
    if (errno == ERANGE) {
        strcpy(buff, "Directory Path too large");
    }

    send_cmd(fd, buff, sizeof(buff));

    return 0;
}
