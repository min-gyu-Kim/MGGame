#include <stdio.h>

#include <pthread.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <liburing.h>
#include <string.h>

int32_t g_listenSockFd;
io_uring g_ring;

bool InitListenSocket();
void Clear();

char recvBfr[1024];
char sendBfr[1024];

int main()
{
    if(false == ::InitListenSocket())
    {
        Clear();
        return -1;
    }

    int result = io_uring_queue_init(16, &g_ring, 0);
    if(result < 0)
    {
        fprintf(stderr, "io_uring_queue_init() error\n"
                        "error string : %s\n",
                strerror(-result)
        );
        Clear();
        return -1;
    }

    io_uring_sqe* sqe = io_uring_get_sqe(&g_ring);
    if(nullptr == sqe)
    {
        fprintf(stderr, "io_uring_get_sqe() error\n"
                        "sqe value is nullptr\n");
        Clear();
        return -1;
    }

    io_uring_prep_multishot_accept(sqe, g_listenSockFd, nullptr, nullptr, 0);
    io_uring_sqe_set_data(sqe, &g_listenSockFd);
    result = io_uring_submit(&g_ring);
    if(result != 1)
    {
        fprintf(stderr, "io_uring_submit() error\n"
                        "error string : %s\n",
                strerror(-result)
        );
        Clear();
        return -1;
    }

    while(true)
    {
        io_uring_cqe* cqe;
        result = io_uring_wait_cqe(&g_ring, &cqe);
        if(result < 0)
        {
            if(result == -EINTR)
                continue;

            fprintf(stderr, "io_uring_wait_cqe() error\n"
                            "error string : %s\n",
                    strerror(-result)
            );
            Clear();
            return -1;
        }

        void* userData = io_uring_cqe_get_data(cqe);
        if(userData == &g_listenSockFd)
        {
            if(cqe->res < 0)
            {
                fprintf(stderr, "accept() error\n"
                                "error string : %s\n",
                        strerror(-cqe->res));
                continue;
            }
            int clientFd = cqe->res;
            sockaddr clientAddr{};
            socklen_t addrSize = sizeof(clientAddr);
            getpeername(clientFd, &clientAddr, &addrSize);

            char ipAddress[32];
            if(clientAddr.sa_family != AF_INET)
            {
                printf("not supported IPv6\n");
                close(clientFd);
                io_uring_cqe_seen(&g_ring, cqe);
                continue;
            }

            inet_ntop(AF_INET, &((sockaddr_in*)&clientAddr)->sin_addr, ipAddress, 32);
            printf("Accept()! %s:%d, clientFd : %d\n", ipAddress, ntohs(((sockaddr_in*)&clientAddr)->sin_port), clientFd);

            sqe = io_uring_get_sqe(&g_ring);
            if(sqe == NULL)
            {
                fprintf(stderr, "io_uring_get_sqe() return NULL\n");
                break;
            }

            io_uring_prep_recv(sqe, clientFd, recvBfr, 1024, 0);
            io_uring_sqe_set_data64(sqe, clientFd);
            io_uring_submit(&g_ring);
        }
        else    // recv
        {
            unsigned long long clientFd = cqe->user_data;
            bool isSend =  (clientFd & (1ULL << 32)) > 0;
            clientFd = clientFd & ((1ULL << 32) - 1);

            if(isSend)
                printf("send Ok\n");

            if(cqe->res < 0)
            {
                if(false == isSend)
                {
                    fprintf(stderr, "recv() error\n"
                                    "error string : %s, fd : %d\n",
                            strerror(-cqe->res), (int)clientFd
                    );
                }
                else
                {
                    fprintf(stderr, "send() error\n"
                                    "error string : %s, fd : %d\n",
                            strerror(-cqe->res), (int)clientFd
                    );
                }
                close(clientFd);
            }
            else if(cqe->res == 0)
            {
                printf("close socket %d, %d\n", (int)clientFd, isSend);
                close(clientFd);
            }
            else if(isSend == false)
            {
                printf("%s : %d\n", recvBfr, cqe->res);

                sqe = io_uring_get_sqe(&g_ring);
                if(sqe == NULL)
                {
                    fprintf(stderr, "io_uring_get_sqe() return NULL\n");
                    break;
                }

                memcpy(sendBfr, recvBfr, cqe->res);

                io_uring_prep_recv(sqe, clientFd, recvBfr, 1024, 0);
                io_uring_sqe_set_data64(sqe, clientFd);

                sqe = io_uring_get_sqe(&g_ring);
                if(sqe == NULL)
                {
                    fprintf(stderr, "io_uring_get_sqe() return NULL\n");
                    break;
                }
                io_uring_prep_send(sqe, clientFd, sendBfr, cqe->res, 0);
                unsigned long long data = (1ULL << 32) | (unsigned long long)clientFd;
                io_uring_sqe_set_data64(sqe, data);

                io_uring_submit(&g_ring);
            }
            else
            {
                //
            }
        }

        io_uring_cqe_seen(&g_ring, cqe);
    }

    Clear();

    return 0;
}

bool InitListenSocket()
{
    g_listenSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(g_listenSockFd == -1)
    {
        printf("socket() failed. result : %d\n", errno);
        return false;
    }

    int option = 1;
    setsockopt(g_listenSockFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = 0;
    serverAddr.sin_port = htons(4000);

    int32_t result = bind(g_listenSockFd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(-1 == result)
    {
        printf("bind() failed. result : %d\n", errno);
        return false;
    }

    result = listen(g_listenSockFd, 100);
    if(-1 == result)
    {
        printf("listen() failed. result : %d\n", errno);
        return false;
    }

    return true;
}

void Clear()
{
    if(g_listenSockFd > 0)
    {
        close(g_listenSockFd);
        g_listenSockFd = -1;
    }

    io_uring_queue_exit(&g_ring);
}
