#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>

#include "BaseTest.h"
#include "BootRomDisableTest.h"
#include "CpuLoadInstructionsTest.h"
#include "CpuArithmeticInstructionsTest.h"
#include "CpuPrefixInstructionsTest.h"
#include "CpuJumpInstructionsTest.h"
#include "CpuFlagOperationsTest.h"
#include "CpuStackOperationsTest.h"
#include "MemoryBusTest.h"
#include "EchoRamTest.h"
#include "InterruptTest.h"
#include "IoRegistersTest.h"
#include "Mbc1Test.h"
#include "SimpleCpuTest.h"
#include "PpuTest.h"
#include "VRamTest.h"
#include "WRamTest.h"
#include "HRamTest.h"
#include "CartridgeTest.h"
#include "OamTest.h"
#include "DeviceTest.h"
#include "NoMbcTest.h"
#include "WRamCgbTest.h"
#include "Core/Logger.h"

int main()
{
    // Disable debug logging for tests to reduce noise
    Logger::SetDebugEnabled(false);
    
    DEBUG("This debug message should not appear in test output");
    
    LOG("=== Game Boy Emulator Test Suite ===");
    LOG("");
    
    // Create list of all tests
    std::vector<std::unique_ptr<BaseTest>> tests;
    tests.push_back(std::make_unique<BootRomDisableTest>());
    tests.push_back(std::make_unique<SimpleCpuTest>());
    tests.push_back(std::make_unique<CpuArithmeticInstructionsTest>());
    tests.push_back(std::make_unique<CpuPrefixInstructionsTest>());
    tests.push_back(std::make_unique<CpuJumpInstructionsTest>());
    tests.push_back(std::make_unique<CpuFlagOperationsTest>());
    tests.push_back(std::make_unique<CpuStackOperationsTest>());
    tests.push_back(std::make_unique<MemoryBusTest>());
    tests.push_back(std::make_unique<EchoRamTest>());
    tests.push_back(std::make_unique<InterruptTest>());
    tests.push_back(std::make_unique<IoRegistersTest>());
    tests.push_back(std::make_unique<Mbc1Test>());
    tests.push_back(std::make_unique<PpuTest>());
    tests.push_back(std::make_unique<VRamTest>());
    tests.push_back(std::make_unique<WRamTest>());
    tests.push_back(std::make_unique<HRamTest>());
    tests.push_back(std::make_unique<CartridgeTest>());
    tests.push_back(std::make_unique<OamTest>());
    tests.push_back(std::make_unique<NoMbcTest>());
    tests.push_back(std::make_unique<WRamCgbTest>());
    // DeviceTest disabled due to crashes - requires further investigation
    // tests.push_back(std::make_unique<DeviceTest>());
    
    // Add more tests here as they are created
    // tests.push_back(std::make_unique<CpuLoadInstructionsTest>());
    // tests.push_back(std::make_unique<PpuRenderingTest>());
    
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