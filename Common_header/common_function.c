#include "rw.c"      //included read write header file.
#include "socket.h"  //included socket header file for TCP.

bool check_file(char *buff) {
    // function to check if file is present or not
    char dir_path[1024];
    printf("Checking %s on local host\n", buff);
    DIR *d;
    struct dirent *dir;
    d = opendir(
        getcwd(dir_path, sizeof(dir_path)));  // current directory opened
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            // for type file in the directory.
            if (dir->d_type == DT_REG) {
                // check name
                if (strcmp(dir->d_name, buff) == 0) {
                    closedir(d);  // found file, so close directory
                    printf("%s found\n", buff);
                    return true;
                }
            }
        }
    }
    closedir(d);  // else not found, so close directory.
    printf("%s not found\n", buff);
    return false;
}

long int recv_file_size(int fd) {  // function to receive file size.
    char size_file[11];
    recv_cmd(fd, size_file, sizeof(size_file));  // recv_cmd function to receive
                                                 // file size from buffer (fd).
    long int file_size = atol(size_file);
    printf("\nSize of requested file is: %ld bytes\n", file_size);
    return file_size;
}

int send_file_size(int fd, long int file_size) {  // function to send file size.
    char size_file[11];
    sprintf(size_file, "%010ld", file_size);
    send_cmd(fd, size_file,
             sizeof(size_file));  // send_cmd function to write/send file size
                                  // from buffer (fd).
    printf("File size sent\n");
    return 0;
}

int send_confirm(int fd, bool option) {
    // function to send confirm/acknowledgement.
    char arr[2] = "0";
    // if option is "true" -> "YES confirm" OR "false" -> "NO confirm".
    if (option) {
        arr[0] = '1';
    }
    int bytes_written;
send_confirm_again:
    bytes_written = write(
        fd, arr,
        sizeof(arr));  // write confirm message(true/false) in the buffer(fd).
    if (bytes_written < 0) {
        // if server waits for something else, then go back to label
        // send_confirm_again and proceed.
        if (errno == EINTR) {
            goto send_confirm_again;
        }
    }
    return 0;
}

bool recv_confirm(int fd) {
    // function to recv confirm/acknowledgement.
    char arr[2];
    int bytes_read;
recv_again:
    // read confirm message from buffer(fd).
    bytes_read = read(fd, arr, sizeof(arr));

    if (bytes_read == 0) {
        return 0;
    }
    if (bytes_read < 0) {
        // if server waits for something else, then go
        // back to label recv_again and proceed
        if (errno == EINTR) {
            bytes_read = 0;
            goto recv_again;
        } else {
            return -1;
        }
    }
    return (arr[0] == '1');  // return true if confirm received true.
}

int recv_file(int fd, char *ptr, long int size) {
    // function to receive file contents.
    char buff[1024];
    strcpy(buff, ptr);

    // open file with filename in buffer(buff).
    int fp = open(buff, O_RDWR | O_CREAT, 0777);
    if (!fp) {
        // if file could not be opened.
        printf("File could not be opened\n");
        close(fp);
        return 0;
    }
    readn(fd, fp, size);  // readn_function in rw.c to read file.
    close(fp);
    return 0;
}

int send_file(int fd, char *ptr) {  // function to send file contents.
    char temp[2] = "1";
    char buff[1024];
    strcpy(buff, ptr);

    int fp = open(buff, O_RDONLY);  // open file from which data is to be sent.
    if (!fp) {                      // if file could not be opened.
        send_confirm(fd, false);    // send confirm as "false" if file opened.
        printf("%s could not be openend\n", buff);
        return -1;
    }
    send_confirm(fd, true);  // send confirm as "true" if file opened.
    struct stat stats;
    fstat(fp, &stats);
    long int file_size = stats.st_size;
    printf("%ld\n", file_size);
    if (file_size > 1000000000) {
        // if file size is greater than 1GB.
        send_confirm(fd, false);  //"false" if file size if large.
        printf("File too Large. Aborting Sending File.\n");
        close(fp);
        return -1;
    }
    send_confirm(fd, true);  //"true" if file is less than or equal to 1GB.
    send_file_size(fd, file_size);     // send file size.
    recv_cmd(fd, temp, sizeof(temp));  // receive ack.

    printf("Sending File ...\n");
    long int k = writen(fd, fp);  // write file in the tcp packets.
    close(fp);
    printf("File Sent %ld\n\n", k);
    return 0;
}

int send_one_file(int fd, char *name) {
    // function to send one file in the mget/mput option.
    char buff[1024];
    char temp[2] = "1";
    strcpy(buff, name);

    send_cmd(fd, buff, sizeof(buff));  // send cmd to send ack.
    printf("%s\n", buff);
    if (!recv_confirm(fd)) {
        return 0;
    }

    if (send_file(fd, buff) == 0) {        // if file sent.
        recv_cmd(fd, temp, sizeof(temp));  // receive ack.
    }

    return 0;
}

int recv_one_file(int fd) {
    // function to receive one file in mget/mput option.
    char buff[1024];
    char temp[2] = "4";

    recv_cmd(fd, buff, sizeof(buff));
    if (check_file(buff)) {
        // check if file exists.
        char option;

    confirm_recv:
        printf("The file %s already exists. Do you wish to overwrite(y/n)? : ",
               buff);
        scanf("\n%c", &option);

        if (option == 'N' || option == 'n') {  // if no then.
            send_confirm(fd, false);
            return 0;
        } else if (option == 'Y' || option == 'y') {  // if yes then.
            send_confirm(fd, true);
        } else {  // else not recognized input.
            printf("Option not recognized.\n");
            goto confirm_recv;
        }
    } else {  // else send_confirm true.
        send_confirm(fd, true);
    }
    if (!recv_confirm(fd)) {
        // if recv-confirm is false then file must have
        // not been opened.
        printf("File could not be opened\n");
        return 0;
    }
    if (!recv_confirm(fd)) {
        // if second recv-confirm is false then file must
        // have a large size than 1GB.
        printf("File size too large, try again later\n");
        return 0;
    }
    // receive file size and then contents.
    long int file_size = recv_file_size(fd);
    send_cmd(fd, temp, sizeof(temp));

    recv_file(fd, buff, file_size);
    send_cmd(fd, temp, sizeof(temp));
    return 0;
}

int put_one_file(
    int fd,
    char *name) {  // function to put one file in the server/mput option.
    char buff[1024];
    char temp[2] = "1";
    strcpy(buff, name);

    send_cmd(fd, buff, sizeof(buff));  // send ack first.
    printf("%s\n", buff);
    if (recv_confirm(fd)) {
        char option;

    put_file_confirm:
        printf("The file %s already exists. Do you want to overwrite(y/n)? : ",
               buff);
        scanf("\n%c",
              &option);  // if file exists and user wants to over-write or not.
        if (option == 'N' ||
            option == 'n') {  // if no then send_confirm as false.
            send_confirm(fd, false);
            recv_cmd(fd, temp, sizeof(temp));
            return 0;
        } else if (option == 'Y' ||
                   option == 'y') {  // if yes then send_confirm as true.
            send_confirm(fd, true);
            recv_cmd(fd, temp, sizeof(temp));
        } else {  // else option not recognized.
            printf("Option not recognized.\n");
            goto put_file_confirm;  // go back and ask again.
        }
    } else {  // if confirm not received.
        send_confirm(fd, true);
        recv_cmd(fd, temp, sizeof(temp));
    }

    if (send_file(fd, buff) == 0) {  // send file finally.
        recv_cmd(fd, temp, sizeof(temp));
    }
    return 0;
}

int recv_put_one_file(
    int fd) {  // function to receive put one file in the buffer.
    char buff[1024];
    char temp[2] = "4";

    recv_cmd(fd, buff, sizeof(buff));  // recv cmd first.
    if (check_file(buff)) {            // check if file exists.
        send_confirm(fd, true);        // send confirm as true.
    } else {                           // else send confirm as false.
        send_confirm(fd, false);
    }

    if (recv_confirm(fd)) {  // if receive confirm fd then send_cmd else return.
        send_cmd(fd, temp, sizeof(temp));
    } else {
        send_cmd(fd, temp, sizeof(temp));
        return 0;
    }
    if (!recv_confirm(fd)) {  // if receive confirm false then file must have
                              // not be opened.
        printf("File could not be opened\n");
        return 0;
    }
    if (!recv_confirm(fd)) {  // if second receive confirm false then file size
                              // must be largeer than 1GB.
        printf("File size too large, try again later\n");
        return 0;
    }
    long int file_size = recv_file_size(fd);  // receive file size .
    send_cmd(fd, temp, sizeof(temp));

    recv_file(fd, buff,
              file_size);  // receive file function to receive file contents.
    send_cmd(fd, temp, sizeof(temp));
    return 0;
}

void get_filename_ext(char *filename, char *extbuff) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        strcpy(extbuff, "");
    else
        strcpy(extbuff, dot + 1);
}
