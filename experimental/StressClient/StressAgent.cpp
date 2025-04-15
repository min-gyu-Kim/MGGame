#include "StressAgent.hpp"
#include <assert.h>
#include <process.h>
#include <WS2tcpip.h>

#pragma comment(lib, "WS2_32.lib")

typedef struct alignas(64) stThreadContext
{
    std::int32_t maxRTT;
    std::int32_t numConnection;
    std::vector<WSAPOLLFD> pollFd;
} ThreadContext;

namespace
{
std::vector<ThreadContext> g_contexts;
thread_local ThreadContext* t_context;
}

#pragma pack(push, 1)
typedef struct stEchoPacket
{
    std::int16_t pakcetSize;
    std::uint64_t sendTime;
} EchoPacket;
#pragma pack(pop)

StressAgent::StressAgent(std::int16_t numClients, std::int16_t numThreads, const std::string_view serverIP, std::uint16_t serverPort) :
    m_numClients(numClients),
    m_numThreads(numThreads),
    m_isRunning(false)
{
    assert(m_numClients > 0);
    assert(m_numThreads > 0);

    WSADATA data{};
    WSAStartup(MAKEWORD(2, 2), &data);

    QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);

    addrinfo* pAddrInfo = nullptr;
    addrinfo addrHint{};
    addrHint.ai_family = AF_INET;
    addrHint.ai_socktype = SOCK_STREAM;
    addrHint.ai_protocol = IPPROTO_TCP;

    std::int32_t result = getaddrinfo(serverIP.data(), nullptr, &addrHint, &pAddrInfo);
    if(0 != result)
    {
        printf("getaddrinfo error code : %d\n", result);
        throw std::exception();
    }

    if(pAddrInfo == nullptr)
    {
        printf("getaddrinfo() addrinfo is null\n");
        throw std::exception();
    }

    memcpy(&m_serverAddr, pAddrInfo->ai_addr, sizeof(sockaddr_in));
    m_serverAddr.sin_port = htons(serverPort);

    freeaddrinfo(pAddrInfo);
}

StressAgent::~StressAgent()
{
    for(const auto& hThread : m_threads)
    {
        WaitForSingleObject(hThread, INFINITE);
    }

    WSACleanup();
}

void StressAgent::Run()
{
    m_isRunning = true;
    for(std::int32_t idx = 0; idx < m_numThreads; ++idx)
    {
        std::uint32_t threadID;
        HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, &StressAgent::workerThread, this, 0, &threadID);
        assert(INVALID_HANDLE_VALUE != hThread);

        m_threads.push_back(hThread);
    }

    while(true)
    {
        // print stat per seconds
        std::int32_t idx = 0;
        for(auto& context : g_contexts)
        {
            printf("[%d] maxRTT : %lfms\n", idx, ((double)context.maxRTT / m_frequency) * 1000.0);
            printf("[%d] numConnection : %d\n", idx, context.numConnection);

            context.maxRTT = 0;
            idx++;
        }
        Sleep(1000);
    }
}

std::uint32_t StressAgent::workerThread(void* args)
{
    assert(args != nullptr);
    ThreadContext& context = g_contexts.emplace_back();
    ZeroMemory(&context, sizeof(ThreadContext));
    t_context = &context;

    t_context->maxRTT = 0;

    StressAgent* agent = (StressAgent*)args;
    agent->runThreadFunc();

    return 0;
}

void StressAgent::runThreadFunc()
{
    std::int32_t numConnect = connectStage();
    if(numConnect > 0)
    {
        printf("[%lu] Num Connect: %d\n", GetCurrentThreadId(), numConnect);
        t_context->numConnection = numConnect;
    }
    else
    {
        printf("[%lu] Zero Connect, Shutdown!\n", GetCurrentThreadId());
        return;
    }

    while(m_isRunning)
    {
        std::int32_t pollResult = WSAPoll(t_context->pollFd.data(), t_context->pollFd.size(), INFINITE);
        if(SOCKET_ERROR == pollResult)
        {
            printf("[%lu] WSAPoll() error code : %d\n", GetCurrentThreadId(), WSAGetLastError());
            return;
        }

        for(std::int32_t idx = 0; idx < t_context->pollFd.size(); ++idx)
        {
            const std::int16_t revents = t_context->pollFd[idx].revents;
            if(POLLRDNORM & revents)
            {
                onRecv(idx);
            }
            else if(POLLHUP & revents)
            {
                closeConnection(idx);
            }
        }
    }

    for(std::int32_t idx = 0; idx < t_context->pollFd.size(); ++idx)
    {
        closeConnection(idx);
    }
    t_context->pollFd.clear();
}

std::int32_t StressAgent::connectStage()
{
    std::int32_t numConnection = 0;

    for(std::int32_t i = 0; i < m_numClients; ++i)
    {
        SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(INVALID_SOCKET == client)
            continue;

        std::int32_t connectResult = connect(client, (const sockaddr*)&m_serverAddr, sizeof(sockaddr_in));
        if(0 != connectResult)
        {
            continue;
        }

        numConnection++;

        WSAPOLLFD fd{};
        fd.fd = client;
        fd.events = POLLRDNORM;
        t_context->pollFd.push_back(fd);

        sendEchoMessage(i);
    }

    return numConnection;
}

bool StressAgent::sendEchoMessage(std::int32_t index)
{
    assert(0 <= index && index < t_context->pollFd.size());

    WSAPOLLFD& fd = t_context->pollFd[index];

    EchoPacket packet{};
    packet.pakcetSize = sizeof(EchoPacket);
    QueryPerformanceCounter((LARGE_INTEGER*)&packet.sendTime);

    std::int32_t sendResult = send(fd.fd, (char*)&packet, sizeof(EchoPacket), 0);
    if(sendResult != sizeof(EchoPacket))
    {
        closeConnection(index);
        return false;
    }

    return true;
}

void StressAgent::onRecv(std::int32_t index)
{
    assert(0 <= index && index < t_context->pollFd.size());

    WSAPOLLFD& fd = t_context->pollFd[index];
    EchoPacket recvPacket{};
    std::int32_t recvResult = recv(fd.fd, (char*)&recvPacket, sizeof(recvPacket), 0);
    if(recvResult == 0)
    {
        closeConnection(index);
        return;
    }
    else if(recvResult < 0)
    {
        printf("recv error code : %d\n", WSAGetLastError());
        closeConnection(index);
        return;
    }
    else if(recvResult != sizeof(EchoPacket))   //TODO: change ringbuffer
    {
        printf("recvResult != %llu\n", sizeof(EchoPacket));
        closeConnection(index);
        return;
    }

    std::int64_t curTick;
    QueryPerformanceCounter((LARGE_INTEGER*)&curTick);
    std::int64_t rtt = curTick - recvPacket.sendTime;

    if(t_context->maxRTT < rtt)
    {
        t_context->maxRTT = rtt;
    }

    sendEchoMessage(index);
}

void StressAgent::closeConnection(std::int32_t index)
{
    assert(0 <= index && index < t_context->pollFd.size());

    WSAPOLLFD& fd = t_context->pollFd[index];
    closesocket(fd.fd);
    fd.fd = INVALID_SOCKET;
    fd.events = 0;

    t_context->numConnection--;
}
