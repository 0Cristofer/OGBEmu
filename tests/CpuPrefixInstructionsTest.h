#pragma once

#include "BaseTest.h"
#include "TestCpu.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/WRam.h"
#include "Emulator/Memory/WRamCgb.h"
#include "Emulator/Memory/EchoRam.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/HRam.h"
#include "Emulator/Memory/Bus.h"
#include <memory>
#include <vector>

class CpuPrefixInstructionsTest : public BaseTest
{
public:
    CpuPrefixInstructionsTest();

    void Setup() override;
    void Run() override;

private:
    // Test categories
    void TestRotateLeftCircular();
    void TestRotateRightCircular();
    void TestRotateLeft();
    void TestRotateRight();
    void TestShiftLeftArithmetic();
    void TestShiftRightArithmetic();
    void TestShiftRightLogical();
    void TestSwap();
    void TestBitOperations();
    void TestSetOperations();
    void TestResetOperations();
    
    // Helper methods
    void WriteProgramToCartridge(const std::vector<byte>& program);
    void WriteProgram(const std::vector<byte>& program);
    void ExecuteProgram(int steps);
    void VerifyMemoryValue(word address, byte expected, const char* description);
    void VerifyRegisterValue(byte expected, const char* description);
    
    // Memory components
    std::unique_ptr<BootRom> _bootRom;
    std::unique_ptr<Cartridge> _cartridge;
    std::unique_ptr<VRam> _vRam;
    std::unique_ptr<WRam> _wRam;
    std::unique_ptr<WRamCgb> _wRamCgb;
    std::unique_ptr<EchoRam> _echoRam;
    std::unique_ptr<Oam> _oam;
    std::unique_ptr<IoRegisters> _ioRegisters;
    std::unique_ptr<HRam> _hRam;
    std::unique_ptr<Bus> _bus;
    std::unique_ptr<TestCpu> _cpu;
    
    std::vector<byte> _bootRomData;
    std::vector<byte> _cartridgeData;
};