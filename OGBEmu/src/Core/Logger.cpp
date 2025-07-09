#include "Logger.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string GetTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::Log(const std::string& log)
{
    static bool firstCall = true;
    static std::ofstream logFile;
    
    if (firstCall)
    {
        logFile.open("emulator_log.txt", std::ios::out); // Overwrite mode
        if (logFile.is_open())
        {
            logFile << "=== OGBEmu Log Started at " << GetTimestamp() << " ===" << '\n';
            logFile.flush();
        }
        firstCall = false;
    }
    
    std::string timestampedLog = "[" + GetTimestamp() + "] " + log;
    
    // Always output to console
    std::cout << timestampedLog << '\n';
    
    // Also output to file if available
    if (logFile.is_open())
    {
        logFile << timestampedLog << '\n';
        logFile.flush();
    }
}

void Logger::DebugBreakLog(const std::string& log)
{
    Log(log);
}
