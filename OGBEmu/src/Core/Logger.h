#pragma once

#include <string>
#include <sstream>

class Logger
{
public:
    static void Log(const std::string& log);
    static void DebugBreakLog(const std::string& log);
};

#define LOG(A) Logger::Log((std::stringstream() << A).str())  // NOLINT(bugprone-macro-parentheses)
#define DEBUGBREAKLOG(A) Logger::DebugBreakLog((std::stringstream() << A).str())  // NOLINT(bugprone-macro-parentheses)
