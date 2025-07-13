#include "JoypadTest.h"
#include "Core/Logger.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>

JoypadTest::JoypadTest() : BaseTest("Joypad Test")
{
}

void JoypadTest::Run()
{
    LOG("  Testing Joypad functionality...");
    
    TestInitialState();
    TestDirectionKeys();
    TestButtonKeys();
    TestKeySelection();
    TestInterruptGeneration();
    TestMultipleKeys();
    TestKeyRelease();
    
    LOG("  Joypad test completed successfully!");
}

void JoypadTest::Setup()
{
    LOG("  Setting up Joypad test...");
    
    _ioRegisters = std::make_unique<IoRegisters>();
    _joypad = std::make_unique<Joypad>(_ioRegisters.get());
    
    LOG("  Joypad test setup complete!");
}

void JoypadTest::TestInitialState()
{
    LOG("    Testing initial state...");
    
    // P1 register should start at 0xFF (all buttons unpressed)
    AssertP1Register(0xFF, "Initial P1 state");
    
    // Clear interrupt flag for testing
    ClearInterruptFlag();
    AssertInterruptFlag(false, "Initial interrupt state");
    
    LOG("    Initial state test passed!");
}

void JoypadTest::TestDirectionKeys()
{
    LOG("    Testing direction keys...");
    
    // Select direction keys (clear bit 4)
    WriteP1Register(0xEF); // 11101111 - select direction keys
    AssertP1Register(0xEF, "Direction keys selected");
    
    // Press Right (should clear bit 0)
    _joypad->SetButtonState(Joypad::Right, true);
    _joypad->Update();
    AssertP1Register(0xEE, "Right button pressed"); // 11101110
    
    // Press Left (should clear bit 1)
    _joypad->SetButtonState(Joypad::Left, true);
    _joypad->Update();
    AssertP1Register(0xEC, "Right and Left pressed"); // 11101100
    
    // Press Up (should clear bit 2)
    _joypad->SetButtonState(Joypad::Up, true);
    _joypad->Update();
    AssertP1Register(0xE8, "Right, Left, and Up pressed"); // 11101000
    
    // Press Down (should clear bit 3)
    _joypad->SetButtonState(Joypad::Down, true);
    _joypad->Update();
    AssertP1Register(0xE0, "All direction keys pressed"); // 11100000
    
    // Release all direction keys
    _joypad->SetButtonState(Joypad::Right, false);
    _joypad->SetButtonState(Joypad::Left, false);
    _joypad->SetButtonState(Joypad::Up, false);
    _joypad->SetButtonState(Joypad::Down, false);
    _joypad->Update();
    AssertP1Register(0xEF, "All direction keys released");
    
    LOG("    Direction keys test passed!");
}

void JoypadTest::TestButtonKeys()
{
    LOG("    Testing button keys...");
    
    // Select button keys (clear bit 5)
    WriteP1Register(0xDF); // 11011111 - select button keys
    AssertP1Register(0xDF, "Button keys selected");
    
    // Press A (should clear bit 0)
    _joypad->SetButtonState(Joypad::A, true);
    _joypad->Update();
    AssertP1Register(0xDE, "A button pressed"); // 11011110
    
    // Press B (should clear bit 1)
    _joypad->SetButtonState(Joypad::B, true);
    _joypad->Update();
    AssertP1Register(0xDC, "A and B pressed"); // 11011100
    
    // Press Select (should clear bit 2)
    _joypad->SetButtonState(Joypad::Select, true);
    _joypad->Update();
    AssertP1Register(0xD8, "A, B, and Select pressed"); // 11011000
    
    // Press Start (should clear bit 3)
    _joypad->SetButtonState(Joypad::Start, true);
    _joypad->Update();
    AssertP1Register(0xD0, "All button keys pressed"); // 11010000
    
    // Release all button keys
    _joypad->SetButtonState(Joypad::A, false);
    _joypad->SetButtonState(Joypad::B, false);
    _joypad->SetButtonState(Joypad::Select, false);
    _joypad->SetButtonState(Joypad::Start, false);
    _joypad->Update();
    AssertP1Register(0xDF, "All button keys released");
    
    LOG("    Button keys test passed!");
}

void JoypadTest::TestKeySelection()
{
    LOG("    Testing key selection...");
    
    // Press both direction and button keys
    _joypad->SetButtonState(Joypad::Right, true);
    _joypad->SetButtonState(Joypad::A, true);
    
    // Select direction keys only
    WriteP1Register(0xEF); // 11101111
    _joypad->Update();
    AssertP1Register(0xEE, "Direction key selection shows Right"); // 11101110
    
    // Select button keys only  
    WriteP1Register(0xDF); // 11011111
    _joypad->Update();
    AssertP1Register(0xDE, "Button key selection shows A"); // 11011110
    
    // Select both (should show both)
    WriteP1Register(0xCF); // 11001111 - both bits 4 and 5 clear
    _joypad->Update();
    byte p1Value = _ioRegisters->Read(AddressConstants::Joypad);
    if ((p1Value & 0x01) != 0) {
        throw std::runtime_error("Both keys selected should show pressed keys");
    }
    
    // Select neither (should show all unpressed)
    WriteP1Register(0xFF); // 11111111
    _joypad->Update();
    AssertP1Register(0xFF, "No keys selected shows all unpressed");
    
    // Clean up
    _joypad->SetButtonState(Joypad::Right, false);
    _joypad->SetButtonState(Joypad::A, false);
    _joypad->Update();
    
    LOG("    Key selection test passed!");
}

void JoypadTest::TestInterruptGeneration()
{
    LOG("    Testing interrupt generation...");
    
    // Clear interrupt flag
    ClearInterruptFlag();
    
    // Press a button (should generate interrupt)
    _joypad->SetButtonState(Joypad::A, true);
    AssertInterruptFlag(true, "Button press generates interrupt");
    
    // Clear interrupt flag
    ClearInterruptFlag();
    
    // Press another button while first is held (should generate interrupt)
    _joypad->SetButtonState(Joypad::B, true);
    AssertInterruptFlag(true, "Second button press generates interrupt");
    
    // Clear interrupt flag
    ClearInterruptFlag();
    
    // Release a button (should not generate interrupt)
    _joypad->SetButtonState(Joypad::A, false);
    AssertInterruptFlag(false, "Button release does not generate interrupt");
    
    // Clean up
    _joypad->SetButtonState(Joypad::B, false);
    
    LOG("    Interrupt generation test passed!");
}

void JoypadTest::TestMultipleKeys()
{
    LOG("    Testing multiple key combinations...");
    
    // Select direction keys
    WriteP1Register(0xEF);
    
    // Press multiple direction keys simultaneously
    _joypad->SetButtonState(Joypad::Up, true);
    _joypad->SetButtonState(Joypad::Right, true);
    _joypad->Update();
    AssertP1Register(0xEA, "Up and Right pressed"); // 11101010
    
    // Switch to button keys while directions are pressed
    WriteP1Register(0xDF);
    _joypad->Update();
    AssertP1Register(0xDF, "Button selection ignores direction keys");
    
    // Press buttons while directions are still held
    _joypad->SetButtonState(Joypad::A, true);
    _joypad->SetButtonState(Joypad::Start, true);
    _joypad->Update();
    AssertP1Register(0xD6, "A and Start pressed"); // 11010110
    
    // Switch back to direction keys
    WriteP1Register(0xEF);
    _joypad->Update();
    AssertP1Register(0xEA, "Direction selection shows held keys");
    
    // Clean up
    _joypad->SetButtonState(Joypad::Up, false);
    _joypad->SetButtonState(Joypad::Right, false);
    _joypad->SetButtonState(Joypad::A, false);
    _joypad->SetButtonState(Joypad::Start, false);
    _joypad->Update();
    
    LOG("    Multiple key combinations test passed!");
}

void JoypadTest::TestKeyRelease()
{
    LOG("    Testing key release behavior...");
    
    // Select direction keys and press some
    WriteP1Register(0xEF);
    _joypad->SetButtonState(Joypad::Left, true);
    _joypad->SetButtonState(Joypad::Down, true);
    _joypad->Update();
    AssertP1Register(0xE5, "Left and Down pressed"); // 11100101
    
    // Release one key
    _joypad->SetButtonState(Joypad::Left, false);
    _joypad->Update();
    AssertP1Register(0xE7, "Only Down pressed"); // 11100111
    
    // Release remaining key
    _joypad->SetButtonState(Joypad::Down, false);
    _joypad->Update();
    AssertP1Register(0xEF, "All keys released");
    
    LOG("    Key release behavior test passed!");
}

void JoypadTest::AssertP1Register(byte expected, const std::string& message)
{
    byte actual = _ioRegisters->Read(AddressConstants::Joypad);
    if (actual != expected)
    {
        std::ostringstream oss;
        oss << message << " - Expected: 0x" << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(expected) << ", Got: 0x" << static_cast<int>(actual);
        throw std::runtime_error(oss.str());
    }
}

void JoypadTest::WriteP1Register(byte value)
{
    _ioRegisters->Write(AddressConstants::Joypad, value);
}

void JoypadTest::AssertInterruptFlag(bool expected, const std::string& message)
{
    byte interruptFlag = _ioRegisters->Read(AddressConstants::InterruptFlag);
    bool actual = (interruptFlag & 0x10) != 0; // Joypad interrupt is bit 4
    if (actual != expected)
    {
        std::ostringstream oss;
        oss << message << " - Expected: " << (expected ? "set" : "clear") 
            << ", Got: " << (actual ? "set" : "clear");
        throw std::runtime_error(oss.str());
    }
}

void JoypadTest::ClearInterruptFlag()
{
    byte interruptFlag = _ioRegisters->Read(AddressConstants::InterruptFlag);
    _ioRegisters->Write(AddressConstants::InterruptFlag, interruptFlag & ~0x10);
}