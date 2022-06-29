#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int socket_init(int *pSocket)
{
    int n;
    int connSockFd, sockFd;
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    printf("[socket]\t\treturned %d: %s\n", sockFd, strerror(errno));
    if (sockFd == -1)
    {
        return SV_FAILED;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(SERVER_PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    // bind
    n = bind(sockFd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
    printf("[bind]\t\treturned %d: %s\n", n, strerror(errno));
    if (n == -1)
        return SV_FAILED;

    // listen
    n = listen(sockFd, LISTEN_BACKLOG);
    printf("[listen]\t\treturned %d: %s\n", n, strerror(errno));
    if (n == -1)
        return SV_FAILED;

    *pSocket = sockFd;

    return SV_SUCCESS;
}

// Accept connection
int acceptClient(int sock){
    int connSock;
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);
    connSock = accept(sock, (struct sockaddr *)&caddr, &clen);
    printf("[accept]\t\treturned %d: %s\n", connSock, strerror(errno));
    if (connSock == -1)
        return SV_FAILED;

    return connSock;
}

int socket_deinit(int *pSocket)
{
    int n;
    n = close(*pSocket);
    printf("[close]\t\treturned %d: %s\n", n, strerror(errno));
    if (n != 0)
        return SV_FAILED;
    return SV_SUCCESS;
}

int createFile(char *path, int offset, int fpos){
    int fd;
    memset(path, 0, sizeof(path));
    sprintf(path, "/tmp/act%d", fpos++);
    mkdir(path, 777);
    sprintf(path, "/image%d.jpg", offset++);
    fd = open(path, O_CREAT | O_APPEND | O_WRONLY);
    return fd;
}

int main(int argc, char *argv[])
{
    int n, numClient = 0;
    int socket, connSockFd;
    char *buffer, path[BUFFER_SIZE];
    buffer = (char *)calloc(BUFFER_SIZE + 1, 1);
    int fd;

    socket_init(&socket);

    // strcpy(buffer, "OK 200 MOTION 7");
    // write(socket, buffer, BUFFER_SIZE);

    // communication
    // step 1: recv request from client

    while (1)
    {
        connSockFd = acceptClient(socket);
        while (1)
        {
            memset(buffer, 0, BUFFER_SIZE);
            n = read(connSockFd, buffer, strlen(REQ_CMD));
            printf("read returned %d , [data: %s]: %s\n", n, buffer, strerror(errno));
            n = strcmp(buffer, REQ_CMD);
            printf("strcmp returned %d: %s\n", n, strerror(errno));
            if (n >= 0)
                break;
        }

        // step 2: send ack to client
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "%s %d", RESP_CMD, RESP_CODE_OK);
        write(connSockFd, buffer, strlen(buffer));
        printf("write returned %d , [data: %s]: %s\n", n, buffer, strerror(errno));

        int num = 0;
        fd = createFile(path, num, numClient);

        // step 3: recv img from client
        while (1) {
            printf("Recive data");
            memset(buffer, 0, BUFFER_SIZE);
            n = read(connSockFd, buffer, strlen(buffer));
            if (n <= 0){
                printf("Client closed");
                socket_deinit(&connSockFd);
                close(fd);
                break;
            }

            buffer[n] = '\0';
            if (strstr(buffer, (DELIMITER)) != NULL){
                //Write file and create new file
                char    *context,
                        *token = strtok_r(buffer, DELIMITER, &context);
                while(token != NULL){
                    write(fd, token, strlen(token));
                    close(fd);
                    token = strtok_r(NULL, DELIMITER, &context);
                    //Create new file name
                    fd = createFile(path, num, numClient);
                }
            }
            else {
                write(fd,  buffer, n);
            }
        }

        socket_deinit(&connSockFd);

        // step 4: send ack to client
        // step 5: send response to client

        // end communication
    }

    free(buffer);

    socket_deinit(&socket);

    return 0;
}