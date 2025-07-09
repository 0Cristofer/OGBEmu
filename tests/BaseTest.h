#pragma once

#include <string>
#include "Core/Definitions.h"

class Bus;
class TestCpu;

class BaseTest
{
public:
    BaseTest(const std::string& testName);
    virtual ~BaseTest() = default;

    virtual void Setup() = 0;
    virtual void Run() = 0;

    void Execute();
    const std::string& GetTestName() const;

protected:
    // Helper method to initialize hardware registers to post-boot state
    void InitializePostBootHardwareState(Bus* bus);

private:
    std::string _testName;
};