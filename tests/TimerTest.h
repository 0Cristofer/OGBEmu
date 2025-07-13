#pragma once

#include "BaseTest.h"
#include "Emulator/Timer.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"
#include <memory>

class TimerTest : public BaseTest
{
public:
    TimerTest();
    void Setup() override;
    void Run() override;

private:
    void TestDividerRegister();
    void TestTimerEnable();
    void TestTimerSpeeds();
    void TestTimerOverflow();
    void TestTimerReset();
    void TestTimerInterrupts();
    
    // Helper methods
    void AssertRegister(word address, byte expected, const std::string& message);
    void WriteRegister(word address, byte value);
    void SimulateCycles(int cycles);
    
    std::unique_ptr<IoRegisters> _ioRegisters;
    std::unique_ptr<Timer> _timer;
};