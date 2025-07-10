#pragma once

#include <string>
#include <sstream>

enum class LogLevel
{
    Debug = 0,
    Log = 1,
    Error = 2
};

class Logger
{
public:
    static void Log(const std::string& log);
    static void Error(const std::string& log);
    static void Debug(const std::string& log);
    
    static void SetDebugEnabled(bool enabled);
    static bool IsDebugEnabled();

private:
    static void LogWithLevel(const std::string& log, LogLevel level);
    static bool s_debugEnabled;
};

#define LOG(A) Logger::Log((std::stringstream() << A).str())  // NOLINT(bugprone-macro-parentheses)
#define ERROR(A) Logger::Error((std::stringstream() << A).str())  // NOLINT(bugprone-macro-parentheses)
#define DEBUG(A) Logger::Debug((std::stringstream() << A).str())  // NOLINT(bugprone-macro-parentheses)
