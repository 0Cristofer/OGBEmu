#include "TimerTest.h"
#include "Core/Logger.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>

TimerTest::TimerTest() : BaseTest("Timer Test")
{
}

void TimerTest::Run()
{
    LOG("  Testing Timer functionality...");
    
    TestDividerRegister();
    TestTimerEnable();
    TestTimerSpeeds();
    TestTimerOverflow();
    TestTimerReset();
    TestTimerInterrupts();
    
    LOG("  Timer test completed successfully!");
}

void TimerTest::Setup()
{
    LOG("  Setting up Timer test...");
    
    _ioRegisters = std::make_unique<IoRegisters>();
    _timer = std::make_unique<Timer>(_ioRegisters.get());
    
    LOG("  Timer test setup complete!");
}

void TimerTest::TestDividerRegister()
{
    LOG("    Testing DIV register...");
    
    // DIV should start at 0
    AssertRegister(AddressConstants::TimerDivider, 0x00, "DIV initial value");
    
    // DIV increments every 256 CPU cycles
    SimulateCycles(256);
    AssertRegister(AddressConstants::TimerDivider, 0x01, "DIV after 256 cycles");
    
    SimulateCycles(256);
    AssertRegister(AddressConstants::TimerDivider, 0x02, "DIV after 512 cycles");
    
    // DIV should overflow at 256
    SimulateCycles(256 * 254); // Total: 256 * 256 cycles
    AssertRegister(AddressConstants::TimerDivider, 0x00, "DIV overflow");
    
    LOG("    DIV register test passed!");
}

void TimerTest::TestTimerEnable()
{
    LOG("    Testing timer enable/disable...");
    
    // Timer should be disabled by default
    AssertRegister(AddressConstants::TimerControl, 0x00, "TAC initial value");
    
    // Set TIMA and TMA
    WriteRegister(AddressConstants::TimerCounter, 0x00);
    WriteRegister(AddressConstants::TimerModulo, 0x50);
    
    // Timer disabled - TIMA should not increment
    SimulateCycles(1024); // Enough cycles for any timer speed
    AssertRegister(AddressConstants::TimerCounter, 0x00, "TIMA disabled");
    
    // Enable timer with speed 00 (1024 cycles)
    WriteRegister(AddressConstants::TimerControl, 0x04); // Enable bit + speed 00
    
    // TIMA should increment after 1024 cycles
    SimulateCycles(1024);
    AssertRegister(AddressConstants::TimerCounter, 0x01, "TIMA enabled after 1024 cycles");
    
    // Disable timer again
    WriteRegister(AddressConstants::TimerControl, 0x00);
    SimulateCycles(1024);
    AssertRegister(AddressConstants::TimerCounter, 0x01, "TIMA disabled again");
    
    LOG("    Timer enable/disable test passed!");
}

void TimerTest::TestTimerSpeeds()
{
    LOG("    Testing timer speeds...");
    
    // Reset timer
    WriteRegister(AddressConstants::TimerCounter, 0x00);
    
    // Test speed 01 (16 cycles)
    WriteRegister(AddressConstants::TimerControl, 0x05); // Enable + speed 01
    SimulateCycles(16);
    AssertRegister(AddressConstants::TimerCounter, 0x01, "Speed 01 (16 cycles)");
    
    // Test speed 10 (64 cycles)
    WriteRegister(AddressConstants::TimerCounter, 0x00);
    WriteRegister(AddressConstants::TimerControl, 0x06); // Enable + speed 10
    SimulateCycles(64);
    AssertRegister(AddressConstants::TimerCounter, 0x01, "Speed 10 (64 cycles)");
    
    // Test speed 11 (256 cycles)
    WriteRegister(AddressConstants::TimerCounter, 0x00);
    WriteRegister(AddressConstants::TimerControl, 0x07); // Enable + speed 11
    SimulateCycles(256);
    AssertRegister(AddressConstants::TimerCounter, 0x01, "Speed 11 (256 cycles)");
    
    // Test speed 00 (1024 cycles)
    WriteRegister(AddressConstants::TimerCounter, 0x00);
    WriteRegister(AddressConstants::TimerControl, 0x04); // Enable + speed 00
    SimulateCycles(1024);
    AssertRegister(AddressConstants::TimerCounter, 0x01, "Speed 00 (1024 cycles)");
    
    LOG("    Timer speeds test passed!");
}

void TimerTest::TestTimerOverflow()
{
    LOG("    Testing timer overflow...");
    
    // Set up timer for overflow
    WriteRegister(AddressConstants::TimerCounter, 0xFF);
    WriteRegister(AddressConstants::TimerModulo, 0xAB);
    WriteRegister(AddressConstants::TimerControl, 0x05); // Enable + speed 01 (16 cycles)
    
    // Clear any existing interrupt flags
    WriteRegister(AddressConstants::InterruptFlag, 0x00);
    
    // Trigger overflow
    SimulateCycles(16);
    
    // TIMA should be reset to TMA value
    AssertRegister(AddressConstants::TimerCounter, 0xAB, "TIMA reset to TMA on overflow");
    
    // Timer interrupt should be set
    byte interruptFlag = _ioRegisters->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x04) == 0) {
        throw std::runtime_error("Timer interrupt not set on overflow");
    }
    
    LOG("    Timer overflow test passed!");
}

void TimerTest::TestTimerReset()
{
    LOG("    Testing timer reset...");
    
    // Set up timer with some values
    WriteRegister(AddressConstants::TimerDivider, 0x55);
    WriteRegister(AddressConstants::TimerCounter, 0x77);
    WriteRegister(AddressConstants::TimerModulo, 0x99);
    WriteRegister(AddressConstants::TimerControl, 0x07);
    
    // Reset timer
    _timer->Reset();
    
    // All registers should be reset to 0
    AssertRegister(AddressConstants::TimerDivider, 0x00, "DIV reset");
    AssertRegister(AddressConstants::TimerCounter, 0x00, "TIMA reset");
    AssertRegister(AddressConstants::TimerModulo, 0x00, "TMA reset");
    AssertRegister(AddressConstants::TimerControl, 0x00, "TAC reset");
    
    LOG("    Timer reset test passed!");
}

void TimerTest::TestTimerInterrupts()
{
    LOG("    Testing timer interrupts...");
    
    // Clear interrupt flag
    WriteRegister(AddressConstants::InterruptFlag, 0x00);
    
    // Set up for multiple overflows
    WriteRegister(AddressConstants::TimerCounter, 0xFE);
    WriteRegister(AddressConstants::TimerModulo, 0x00);
    WriteRegister(AddressConstants::TimerControl, 0x05); // Enable + speed 01
    
    // First overflow
    SimulateCycles(16); // 0xFE -> 0xFF
    SimulateCycles(16); // 0xFF -> overflow -> 0x00
    
    byte interruptFlag = _ioRegisters->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x04) == 0) {
        throw std::runtime_error("First timer interrupt not set");
    }
    
    // Clear interrupt flag manually (normally done by interrupt handler)
    WriteRegister(AddressConstants::InterruptFlag, interruptFlag & ~0x04);
    
    // Second overflow
    WriteRegister(AddressConstants::TimerCounter, 0xFF);
    SimulateCycles(16); // 0xFF -> overflow -> 0x00
    
    interruptFlag = _ioRegisters->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x04) == 0) {
        throw std::runtime_error("Second timer interrupt not set");
    }
    
    LOG("    Timer interrupts test passed!");
}

void TimerTest::AssertRegister(word address, byte expected, const std::string& message)
{
    byte actual = _ioRegisters->Read(address);
    if (actual != expected)
    {
        std::ostringstream oss;
        oss << message << " - Expected: 0x" << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(expected) << ", Got: 0x" << static_cast<int>(actual);
        throw std::runtime_error(oss.str());
    }
}

void TimerTest::WriteRegister(word address, byte value)
{
    _ioRegisters->Write(address, value);
}

void TimerTest::SimulateCycles(int cycles)
{
    _timer->Update(cycles);
}