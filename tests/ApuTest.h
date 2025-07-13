#pragma once

#include "BaseTest.h"
#include "Emulator/Apu.h"
#include "Emulator/Memory/IoRegisters.h"
#include <memory>

class ApuTest : public BaseTest
{
public:
    ApuTest();
    void Setup() override;
    void Run() override;

private:
    void TestInitialState();
    void TestRegisterInitialization();
    void TestEnableDisable();
    void TestFrameSequencer();
    void TestChannel1Registers();
    void TestChannel2Registers();
    void TestChannel3Registers();
    void TestChannel4Registers();
    void TestMasterControl();
    
    // Helper methods
    void AssertRegister(word address, byte expected, const std::string& message);
    void WriteRegister(word address, byte value);
    void SimulateCycles(int cycles);
    
    std::unique_ptr<IoRegisters> _ioRegisters;
    std::unique_ptr<Apu> _apu;
};