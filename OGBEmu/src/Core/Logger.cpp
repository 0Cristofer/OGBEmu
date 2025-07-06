#include "Logger.h"

#include <iostream>
#include <fstream>

void Logger::Log(const std::string& log)
{
    static bool firstCall = true;
    static std::ofstream logFile;
    
    if (firstCall)
    {
        logFile.open("emulator_log.txt", std::ios::out); // Overwrite mode
        firstCall = false;
    }
    
    if (logFile.is_open())
    {
        logFile << log << '\n';
        logFile.flush();
    }
    else
    {
        std::cout << log << '\n';
    }
}

void Logger::DebugBreakLog(const std::string& log)
{
    Log(log);
}
