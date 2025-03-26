#pragma once

#include <cstdint>

namespace core
{
    enum class eLogLevel : uint8_t
    {
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    template <eLogLevel level, class OutputPolicy>
    class Logger : public OutputPolicy
    {
    public:
        explicit Logger(const char* loggerName);
        ~Logger();

        void Log(const eLogLevel logLevel, const char* msg);
    };
}
#include "Logger.ipp"
