ssize_t readn(int fd, int vptr, long int size) {
    // readn function to read file contents from the buffer.
    printf("Recieving file in progress ....\n");
    int fp = vptr;
    long int nleft = size;
    // nleft variable to store size of the file content.
    size_t n;
    ssize_t nread;
    char ptr[1024];
    while (nleft > 0) {  // while nleft>0 ie. file contents still left to sent.
        if ((nread = read(fd, ptr, sizeof(ptr))) < 0) {
            // nread to read content from buffer (fd) into ptr.
            if (errno == EINTR)
                // if interrupt occurs, put nread=0 and start again.
                nread = 0;
            else
                return -1;
        } else if (nread == 0) {  // else if file content is over.
            break;
        }
        n += nread;
        nleft -= nread;
        while (nread > 0) {  // while nread>0 write ptr into fp -> file pointer.
            ssize_t bytes_written = write(fp, ptr, nread);
            nread -= bytes_written;
        }
    }
    printf("Recieving file finished\n");
    return n;
}

ssize_t writen(int fd, int vptr) {
    /* write file content into the buffer
     * INPUT :
     * fd : Socket
     * vptr : file pointer
     */
    size_t nleft, n = 0;
    ssize_t nwritten;
    int fp = vptr;
    char ptr[1024];
    char *pt;

    /* Read the contents of the file into a buffer
     * Send the buffer through the socket
     * Do this while until the file pointer becomes empty
     */
    while (1) {
        memset(ptr, 0x00, 1024);
        int bytes_read = read(fp, ptr, sizeof(ptr));
        if (bytes_read == 0) {
            break;
        }
        if (bytes_read < 0) {
            if (errno == EINTR) {
                bytes_read = 0;
            } else {
                return -1;
            }
        }
        send(fd, ptr, bytes_read, 0);
        n += bytes_read;
    }
    return n;
}

ssize_t send_cmd(int fd, const void *vptr, size_t n) {
    // function to write n bytes of buffer into the socket
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (char *)vptr;
    nleft = n;
    // write as many bytes as can be written at the time
    // continue the above process until all bytes are written
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        } else if (nwritten == 0) {
            break;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n - nleft);
}

ssize_t recv_cmd(int fd, void *vptr, size_t n) {
    // read n bytes into buffer from the socket and return the number of bytes
    // read
    long int nleft = n;
    // nleft variable to store size of the file content.
    ssize_t nread;
    char ptr[1024];
    while (nleft > 0) {  // while nleft>0 ie. file contents still left to sent.
        if ((nread = read(fd, vptr, nleft)) < 0) {
            // nread to read content from buffer (fd) into ptr.
            if (errno == EINTR)
                // if interrupt occurs, put nread=0 and start again.
                nread = 0;
            else
                return -1;
        } else if (nread == 0) {  // else if file content is over.
            break;
        }
        nleft -= nread;
        vptr += nread;
    }
    return n-nleft;
}
