#include "BaseTest.h"

#include <iostream>
#include <exception>
#include "Core/Logger.h"

BaseTest::BaseTest(const std::string& testName) : _testName(testName)
{
}

void BaseTest::Execute()
{
    LOG("Running test: " << _testName);
    
    try
    {
        Setup();
        Run();
        LOG("✓ " << _testName << " PASSED");
    }
    catch (const std::exception& e)
    {
        LOG("✗ " << _testName << " FAILED: " << e.what());
        throw;
    }
    catch (...)
    {
        LOG("✗ " << _testName << " FAILED: Unknown exception");
        throw;
    }
}

const std::string& BaseTest::GetTestName() const
{
    return _testName;
}