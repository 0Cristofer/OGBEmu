#include "Logger.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

bool Logger::s_debugEnabled = true;

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

void Logger::LogWithLevel(const std::string& log, LogLevel level)
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
    
    std::string levelPrefix;
    std::string colorCode;
    std::string resetCode = "\033[0m";
    
    switch (level)
    {
        case LogLevel::Debug:
            levelPrefix = "[DEBUG] ";
            colorCode = "\033[33m"; // Yellow
            break;
        case LogLevel::Log:
            levelPrefix = "[LOG] ";
            colorCode = "\033[34m"; // Blue
            break;
        case LogLevel::Error:
            levelPrefix = "[ERROR] ";
            colorCode = "\033[31m"; // Red
            break;
    }
    
    std::string timestampedLog = "[" + GetTimestamp() + "] " + levelPrefix + log;
    
    // Always output to console with color
    std::cout << colorCode << timestampedLog << resetCode << '\n';
    
    // Also output to file if available
    if (logFile.is_open())
    {
        logFile << timestampedLog << '\n';
        logFile.flush();
    }
}

void Logger::Log(const std::string& log)
{
    LogWithLevel(log, LogLevel::Log);
}

void Logger::Error(const std::string& log)
{
    LogWithLevel(log, LogLevel::Error);
}

void Logger::Debug(const std::string& log)
{
    if (s_debugEnabled)
    {
        LogWithLevel(log, LogLevel::Debug);
    }
}

void Logger::SetDebugEnabled(bool enabled)
{
    s_debugEnabled = enabled;
}

bool Logger::IsDebugEnabled()
{
    return s_debugEnabled;
}
