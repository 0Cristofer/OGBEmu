#pragma once

#include <string>

class BaseTest
{
public:
    BaseTest(const std::string& testName);
    virtual ~BaseTest() = default;

    virtual void Setup() = 0;
    virtual void Run() = 0;

    void Execute();
    const std::string& GetTestName() const;

private:
    std::string _testName;
};