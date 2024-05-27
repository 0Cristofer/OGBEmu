#include "Logger.h"

#include <iostream>

void Logger::Log(const std::string& log)
{
    std::cout << log << '\n';
}

void Logger::DebugBreakLog(const std::string& log)
{
    Log(log);
}
