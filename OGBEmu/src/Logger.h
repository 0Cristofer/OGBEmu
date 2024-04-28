#pragma once

#include <string>
#include <sstream>

class Logger
{
public:
    static void Log(const std::string& log);
};

#define LOG(A) Logger::Log((std::stringstream() << A << '\n').str())  // NOLINT(bugprone-macro-parentheses)
