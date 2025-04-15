#include <cstdint>

#include "StressAgent.hpp"

const std::int16_t NUM_CLIENTS = 1;
const std::int16_t NUM_THREADS = 1;
const char* SERVER_IP = "192.168.0.8";
const std::uint16_t SERVER_PORT = 18001;

int main()
{
    StressAgent agent(NUM_CLIENTS, NUM_THREADS, SERVER_IP, SERVER_PORT);
    agent.Run();
    return 0;
}
