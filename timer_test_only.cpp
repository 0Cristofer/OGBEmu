#include <iostream>
#include "tests/TimerTest.h"
#include "Core/Logger.h"

int main()
{
    // Enable debug logging to see what's happening
    Logger::SetDebugEnabled(false);
    
    LOG("=== Timer Test Only ===");
    
    try {
        TimerTest test;
        test.Execute();
        LOG("✓ Timer test PASSED");
        return 0;
    }
    catch (const std::exception& e) {
        LOG("✗ Timer test FAILED: " << e.what());
        return 1;
    }
}