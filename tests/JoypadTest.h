#pragma once

#include "BaseTest.h"
#include "Emulator/Joypad.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"
#include <memory>

class JoypadTest : public BaseTest
{
public:
    JoypadTest();
    void Setup() override;
    void Run() override;

private:
    void TestInitialState();
    void TestDirectionKeys();
    void TestButtonKeys();
    void TestKeySelection();
    void TestInterruptGeneration();
    void TestMultipleKeys();
    void TestKeyRelease();
    
    // Helper methods
    void AssertP1Register(byte expected, const std::string& message);
    void WriteP1Register(byte value);
    void AssertInterruptFlag(bool expected, const std::string& message);
    void ClearInterruptFlag();
    
    std::unique_ptr<IoRegisters> _ioRegisters;
    std::unique_ptr<Joypad> _joypad;
};