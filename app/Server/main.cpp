#include <stdio.h>

#include <pthread.h>
#include <sys/unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

int32_t listenSockFd;
int32_t epollFd;

bool InitListenSocket();
bool InitSingleEpoll();
void Clear();

int main()
{
    if(false == ::InitListenSocket())
    {
        Clear();
        return -1;
    }

    if(false == InitSingleEpoll())
    {
        Clear();
        return -1;
    }

    while(true)
    {
        epoll_event evts[10];
        int32_t result = epoll_wait(epollFd, evts, 10, -1);
        if(result < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            printf("epoll_wait() failed. result : %d\n", errno);
            break;
        }

        for(int32_t i = 0; i < result; i++)
        {
            if(evts[i].data.fd == listenSockFd)
            {
                int clientFd;
                sockaddr_in clientAddr;
                socklen_t addrSize = sizeof(clientAddr);

                clientFd = accept(listenSockFd, (sockaddr*)&clientAddr, &addrSize);
                if(clientFd < 0)
                {
                    printf("accept() failed. result : %d\n", errno);
                    break;
                }

                int flags = fcntl(clientFd, F_GETFL);
                flags |= O_NONBLOCK;
                int result = fcntl(clientFd, F_SETFL, flags);
                if(-1 == result)
                {
                    printf("fcntl() nonblock mode failed, result : %d\n", errno);
                    close(clientFd);
                    break;
                }

                epoll_event events;
                events.events =EPOLLIN | EPOLLET;
                events.data.fd = clientFd;
                if(epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &events) < 0)
                {
                    printf("epoll_ctl() failed. In Accept, result : %d\n", errno);
                    break;
                }
            }
            else
            {
                int32_t clientFd = evts[i].data.fd;

                char msg[255];
                int32_t result = recv(clientFd, msg, 255, 0);
                if(result < 0)
                {
                    printf("recv() failed. result : %d\n", errno);
                    break;
                }
                if(result == 0)
                {
                    printf("0 recv. close connection\n");
                    break;
                }

                ssize_t sendSize = send(clientFd, msg, result, 0);
                if(sendSize <= 0)
                {
                    printf("send() failed, result : %d\n", errno);
                    break;
                }
            }
        }
    }

    Clear();

    return 0;
}

bool InitListenSocket()
{
    listenSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSockFd == -1)
    {
        printf("socket() failed. result : %d\n", errno);
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = 0;
    serverAddr.sin_port = htons(4000);

    int32_t result = bind(listenSockFd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(-1 == result)
    {
        printf("bind() failed. result : %d\n", errno);
        return false;
    }

    result = listen(listenSockFd, 100);
    if(-1 == result)
    {
        printf("listen() failed. result : %d\n", errno);
        return false;
    }

    return true;
}

bool InitSingleEpoll()
{
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    if(-1 == epollFd)
    {
        printf("epoll_create1() failed. result : %d\n", errno);
        return false;
    }

    epoll_event event{};
    event.data.fd = listenSockFd;
    event.events = EPOLLIN | EPOLLET;

    int32_t result = epoll_ctl(epollFd, EPOLL_CTL_ADD, listenSockFd, &event);
    if(-1 == result)
    {
        printf("epoll_ctl() failed. result : %d\n", errno);
        return false;
    }

    return true;
}

void Clear()
{
    if(listenSockFd > 0)
    {
        close(listenSockFd);
        listenSockFd = -1;
    }

    if(epollFd > 0)
    {
        close(epollFd);
        epollFd = -1;
    }
}
