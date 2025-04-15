#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>

class StressAgent final
{
public:
    StressAgent() = delete;
    StressAgent(std::int16_t numClients, std::int16_t numThreads, const std::string_view serverIP, std::uint16_t serverPort);
    ~StressAgent();

    void Run();

private:
    static std::uint32_t _stdcall workerThread(void* args);
    void runThreadFunc();

    std::int32_t connectStage();
    bool sendEchoMessage(std::int32_t index);
    void onRecv(std::int32_t index);
    void closeConnection(std::int32_t index);

private:
    bool m_isRunning;
    std::int16_t m_numClients;
    std::int16_t m_numThreads;
    std::int64_t m_frequency;
    sockaddr_in m_serverAddr;
    std::vector<HANDLE> m_threads;
};
