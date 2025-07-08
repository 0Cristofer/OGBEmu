#include "BaseTest.h"

#include <iostream>
#include <exception>

BaseTest::BaseTest(const std::string& testName) : _testName(testName)
{
}

void BaseTest::Execute()
{
    std::cout << "Running test: " << _testName << std::endl;
    
    try
    {
        Setup();
        Run();
        std::cout << "✓ " << _testName << " PASSED" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "✗ " << _testName << " FAILED: " << e.what() << std::endl;
        throw;
    }
    catch (...)
    {
        std::cout << "✗ " << _testName << " FAILED: Unknown exception" << std::endl;
        throw;
    }
}

const std::string& BaseTest::GetTestName() const
{
    return _testName;
}