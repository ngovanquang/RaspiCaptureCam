#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int socket_init(int *pSocket)
{
    int connSockFd, sockFd;
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFd == -1)
    {
        perror("socket()");
        return SV_FAILED;
    }
    printf("socket create success!\n");

    struct sockaddr_in saddr, caddr;
    saddr.sin_family        = AF_INET;
    saddr.sin_port          = htons(SERVER_PORT);
    saddr.sin_addr.s_addr   = INADDR_ANY;

    //bind
    if(bind(sockFd, (struct sockadd*)&saddr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind()");
        return SV_FAILED;
    }
    printf("binding...\n");
    //listen
    if(listen(sockFd, LISTEN_BACKLOG) == -1)
    {
        perror("listen()");
        return SV_FAILED;
    }
    printf("listening...\n");
    //accept
    socklen_t clen = sizeof(caddr);
    connSockFd = accept(sockFd, (struct sockaddr*)&caddr, &clen);
    if(connSockFd == -1)
    {
        perror("accept()");
        return SV_FAILED;
    }
    printf("accepted...\n");
    *pSocket = connSockFd;

   return SV_SUCCESS; 
}

int socket_deinit(int *pSocket)
{
    if(close(*pSocket) != 0)
    {
        perror("close()");
        return SV_FAILED;
    }
    return SV_SUCCESS;
}

int main(int argc, char* argv[])
{
    int socket;
    char* buffer;
    buffer = (char*)calloc(BUFFER_SIZE, 1);


    if(SV_FAILED == socket_init(&socket))
    {
        printf("socket_init() failed\n");
    }
    printf("socket_init() successfull!\n");

    strcpy(buffer, "OK 200 MOTION 7"); 
    write(socket, buffer, BUFFER_SIZE);




    free(buffer);

    if(SV_FAILED == socket_deinit(&socket))
    {
        printf("socket_deinit() failed\n");
    }
    printf("socket_deinit() successfull!\n");
    
    return 0;
}