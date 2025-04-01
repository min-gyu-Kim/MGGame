#include <stdio.h>
#include <string.h>

#include <sys/unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#define SERVER_PORT 4000

int main()
{
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == clientFd)
    {
        printf("socket() failed, result : %d\n", errno);
        return -1;
    }

    int result = connect(clientFd, (const sockaddr*)&serverAddr, sizeof(serverAddr));
    if(-1 == result)
    {
        printf("connect() failed, result : %d\n", errno);
        close(clientFd);
        return -1;
    }

    const char* msg = "Hello TCP World!";
    size_t msgLen = strlen(msg) + 1; //null terminate string

    while(true)
    {
        ssize_t sendSize = send(clientFd, msg, msgLen, 0);
        if(sendSize != msgLen)
        {
            printf("sendSize != msgLen, sendSize = %ld\n", sendSize);
            close(clientFd);
            return -1;
        }

        char recvBuffer[255];
        ssize_t recvSize = recv(clientFd, recvBuffer, 255, 0);
        if(recvSize < 0)
        {
            printf("recv() failed, result : %d\n", errno);
            close(clientFd);
            return -1;
        }

        if(recvSize == 0)
        {
            printf("0 recv close!\n");
            close(clientFd);
            return -1;
        }

        if(recvBuffer[recvSize - 1] != '\0')
        {
            printf("Not Echo message!\n");
            close(clientFd);
            return -1;
        }
    }

    return 0;
}
