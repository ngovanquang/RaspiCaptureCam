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

int socket_deinit(int socket)
{
    int n;
    n = close(socket);
    printf("[close]\t\treturned %d: %s\n", n, strerror(errno));
    if (n != 0)
        return SV_FAILED;
    return SV_SUCCESS;
}

int main(int argc, char *argv[])
{
    int n;
    int socket, connSockFd;
    char *img_buffer;
    char *buffer, path[BUFFER_SIZE];
    buffer = (char *)calloc(BUFFER_SIZE + 1, 1);
    img_buffer = (char *) calloc(IMG_BUFFER_LEN, 1);
    memset(path, 0, BUFFER_SIZE);
    int fd;
    char* send_cmd;
    char* img_size_label; 
    send_cmd = (char*)calloc(strlen(SEND_CMD), 1);
    img_size_label = (char*)calloc(strlen(IMG_SIZE_LABEL), 1);
    int byteused, imgnum;


    socket_init(&socket);

    // strcpy(buffer, "OK 200 MOTION 7");
    // write(socket, buffer, BUFFER_SIZE);

    // communication

    int temp = 0;
WAIT_NEWCONNECTION:
    while (1)
    {
        connSockFd = acceptClient(socket);

        /* Step 1: server receive resquest from client 
            if receive message: HANDLE --> goto step 2
            else loop
        */
        do {
            memset(buffer, 0, BUFFER_SIZE);
            n = read(connSockFd, buffer, strlen(REQ_CMD));
            printf("read returned %d , [data: %s]: %s\n", n, buffer, strerror(errno));
            n = strcmp(buffer, REQ_CMD);
            printf("strcmp returned %d: %s\n", n, strerror(errno));

        } while(n != 0);

        /* step 2: send ack(OK 200) to client */
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "%s %d", RESP_CMD, RESP_CODE_OK);
        write(connSockFd, buffer, strlen(buffer));
        printf("write returned %d , [data: %s]: %s\n", n, buffer, strerror(errno));

        /* step 3: recv img from client */

        while (1) {
            printf("Recive data\n");
            memset(buffer, 0, BUFFER_SIZE);
            memset(img_buffer, 0, IMG_BUFFER_LEN);

            // server receive message (SEND filenum BUFFUSED imgsize) from client and send ack to client
            while(1)
            {
                memset(buffer, 0, BUFFER_SIZE);
                n = read(connSockFd, buffer, BUFFER_SIZE);
                printf("read returned: %d: %s\n", n, strerror(errno));

                printf("BUFFER: %s\n", buffer);
                sscanf(buffer, "%s %d %s %d", send_cmd, &imgnum, img_size_label, &byteused);
                // if  
                if(strcmp(send_cmd, SEND_CMD) != 0 || strcmp(img_size_label, IMG_SIZE_LABEL) != 0)
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "%s %d", RESP_CMD, RESP_CODE_FAILE);
                    n = write(connSockFd, buffer, strlen(buffer));
                    printf("write response returned: %d: %s\n", n, strerror(errno));

                } else {
                    // deinit connection sock when byteused = -1 vs imgnum = -1
                    if(byteused == -1 && imgnum == -1) {
                        socket_deinit(connSockFd);
                        goto WAIT_NEWCONNECTION;
                    }
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "%s %d", RESP_CMD, RESP_CODE_OK);
                    n = write(connSockFd, buffer, strlen(buffer));
                    printf("write response returned: %d: %s\n", n, strerror(errno));

                    sprintf(path, "/home/ngoquang/Desktop/demo/hello%d.jpg", imgnum);
                    fd = open(path, O_CREAT | O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
                    printf("open returned %d: %s\n", fd, strerror(errno));
                    break;
                }

            }

            // write img to file
            // vs byteused > BUFFER_SIZE
            while(byteused > BUFFER_SIZE) {
                n = read(connSockFd, buffer, BUFFER_SIZE);
                temp += n;
                printf("read returned: [%d : %d]: %s\n", n, temp, strerror(errno));
                memcpy(img_buffer, buffer, n);
                img_buffer += n;
                byteused -= n;
            }
            // vs byteused < BUFFER_SIZE
            do {
                n = read(connSockFd, buffer, byteused);
                temp += n;
                printf("read returned: [%d : %d]: %s\n", n, temp, strerror(errno));
                memcpy(img_buffer, buffer, n);
                img_buffer += n;
                byteused -= n;
            } while(byteused != 0);

            img_buffer -= temp;
            n = write(fd, img_buffer, temp);
            printf("write file %d returned: %d: %s\n", imgnum, n, strerror(errno));
            n = close(fd);
            printf("close file %d returned: %d: %s\n",imgnum, n, strerror(errno));
            temp = 0;

            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer, "%s %d", RESP_CMD, RESP_CODE_OK);
            n = write(connSockFd, buffer, strlen(buffer));
            printf("write response returned: %d: %s\n", n, strerror(errno));
            //socket_deinit(connSockFd);

            // step 4: send ack to client
            // step 5: send response to client

            // end communication
        }
    }

    free(img_buffer);
    free(img_size_label);
    free(send_cmd);
    free(buffer);

    socket_deinit(socket);

    return 0;
}
