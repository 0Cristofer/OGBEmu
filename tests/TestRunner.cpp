#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>

#include "BaseTest.h"
#include "BootRomDisableTest.h"
#include "CpuLoadInstructionsTest.h"
#include "CpuArithmeticInstructionsTest.h"
#include "CpuJumpInstructionsTest.h"
#include "CpuFlagOperationsTest.h"
#include "CpuStackOperationsTest.h"
#include "SimpleCpuTest.h"
#include "Core/Logger.h"

int main()
{
    LOG("=== Game Boy Emulator Test Suite ===");
    LOG("");
    
    // Create list of all tests
    std::vector<std::unique_ptr<BaseTest>> tests;
    tests.push_back(std::make_unique<BootRomDisableTest>());
    tests.push_back(std::make_unique<SimpleCpuTest>());
    tests.push_back(std::make_unique<CpuArithmeticInstructionsTest>());
    tests.push_back(std::make_unique<CpuJumpInstructionsTest>());
    tests.push_back(std::make_unique<CpuFlagOperationsTest>());
    tests.push_back(std::make_unique<CpuStackOperationsTest>());
    
    // Add more tests here as they are created
    // tests.push_back(std::make_unique<CpuLoadInstructionsTest>());
    // tests.push_back(std::make_unique<PpuRenderingTest>());
    // tests.push_back(std::make_unique<MemoryBusTest>());
    
    int totalTests = static_cast<int>(tests.size());
    int passedTests = 0;
    int failedTests = 0;
    
    LOG("Running " << totalTests << " tests...");
    LOG("");
    
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
        LOG("");
    }
    
    // Print summary
    LOG("=== Test Results ===");
    LOG("Total tests:  " << totalTests);
    LOG("Passed:       " << passedTests);
    LOG("Failed:       " << failedTests);
    LOG("Success rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%");
    
    if (failedTests > 0)
    {
        LOG("");
        LOG("Some tests failed! Check output above for details.");
        return 1;
    }
    else
    {
        LOG("");
        LOG("All tests passed! ✓");
        return 0;
    }
}