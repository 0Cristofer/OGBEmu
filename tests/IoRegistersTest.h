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

class IoRegistersTest : public BaseTest
{
private:
    // Test components
    std::vector<byte> _bootRomData;
    std::vector<byte> _cartridgeData;
    
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
    
    // System components
    std::unique_ptr<Bus> _bus;
    std::unique_ptr<TestCpu> _cpu;

public:
    IoRegistersTest();
    void Setup() override;
    void Run() override;

private:
    // Test methods
    void TestBasicIoRegisterAccess();
    void TestIoRegisterAddressBoundaries();
    void TestLcdControlRegister();
    void TestLcdStatusRegister();
    void TestScrollRegisters();
    void TestLcdYRegister();
    void TestPaletteRegisters();
    void TestWindowRegisters();
    void TestDmaRegister();
    void TestBootRomBankRegister();
    void TestInterruptFlagRegister();
    void TestSpecialBehaviorRegisters();
    
    // Helper methods
    void VerifyRegisterValue(word address, byte expected, const std::string& testName);
    void VerifyRegisterDefault(word address, byte expected, const std::string& registerName);
    void WriteProgramToCartridge(const std::vector<byte>& program);
    void ExecuteInstructions(int count);
};