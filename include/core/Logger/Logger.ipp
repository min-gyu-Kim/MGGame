
namespace core
{
template <eLogLevel level, class OutputPolicy>
Logger<level, OutputPolicy>::Logger(const char* loggerName) :
    OutputPolicy(loggerName)
{

}

template <eLogLevel level, class OutputPolicy>
Logger<level, OutputPolicy>::~Logger() { }

template <eLogLevel level, class OutputPolicy>
void Logger<level, OutputPolicy>::Log(const eLogLevel logLevel, const char* msg)
{
    if(logLevel < level)
    {
        return;
    }

    OutputPolicy::Write(msg);
}
}
