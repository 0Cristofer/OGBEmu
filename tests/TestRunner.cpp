#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>

#include "BaseTest.h"
#include "BootRomDisableTest.h"
#include "CpuLoadInstructionsTest.h"

int main()
{
    std::cout << "=== Game Boy Emulator Test Suite ===" << std::endl;
    std::cout << std::endl;
    
    // Create list of all tests
    std::vector<std::unique_ptr<BaseTest>> tests;
    tests.push_back(std::make_unique<BootRomDisableTest>());
    tests.push_back(std::make_unique<CpuLoadInstructionsTest>());
    
    // Add more tests here as they are created
    // tests.push_back(std::make_unique<CpuArithmeticInstructionsTest>());
    // tests.push_back(std::make_unique<PpuRenderingTest>());
    // tests.push_back(std::make_unique<MemoryBusTest>());
    
    int totalTests = static_cast<int>(tests.size());
    int passedTests = 0;
    int failedTests = 0;
    
    std::cout << "Running " << totalTests << " tests..." << std::endl;
    std::cout << std::endl;
    
    for (auto& test : tests)
    {
        try
        {
            test->Execute();
            passedTests++;
        }
        catch (...)
        {
            failedTests++;
        }
        std::cout << std::endl;
    }
    
    // Print summary
    std::cout << "=== Test Results ===" << std::endl;
    std::cout << "Total tests:  " << totalTests << std::endl;
    std::cout << "Passed:       " << passedTests << std::endl;
    std::cout << "Failed:       " << failedTests << std::endl;
    std::cout << "Success rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%" << std::endl;
    
    if (failedTests > 0)
    {
        std::cout << std::endl;
        std::cout << "Some tests failed! Check output above for details." << std::endl;
        return 1;
    }
    else
    {
        std::cout << std::endl;
        std::cout << "All tests passed! ✓" << std::endl;
        return 0;
    }
}