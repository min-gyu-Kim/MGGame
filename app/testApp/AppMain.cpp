#include <iostream>
#include <cassert>
#include <cstring>

#include <core/Logger/Logger.hpp>

class ConsoleOutputPolicy
{
public:
    ConsoleOutputPolicy(const char* name)
    {
        std::cout << "logger name is " << name << std::endl;
    }

    void Write(const char* msg)
    {
        std::cout << msg << std::endl;
    }
};

class FileOutputPolicy
{
public:
    FileOutputPolicy(const char* name) :
        fp(nullptr)
    {
        assert(name != nullptr);
        fp = fopen(name, "w+");
        assert(fp != nullptr);
    }

    ~FileOutputPolicy()
    {
        assert(fp != nullptr);
        fclose(fp);
    }

    void Write(const char* msg)
    {
        assert(fp != nullptr);
        assert(msg != nullptr);
        fwrite(msg, strlen(msg), 1, fp);
        fflush(fp);
    }

private:
    FILE* fp;
};

int main()
{
    core::Logger<core::eLogLevel::DEBUG, ConsoleOutputPolicy> logger("testLogger");
    logger.Log(core::eLogLevel::INFO, "Hello World!");

    core::Logger<core::eLogLevel::DEBUG, FileOutputPolicy> fileLogger("testFileLogger.txt");
    fileLogger.Log(core::eLogLevel::INFO, "This is File Log.\n");

    return 0;
}

