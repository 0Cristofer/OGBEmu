#pragma once

#include "BaseTest.h"
#include "TestCpu.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/WRam.h"
#include "Emulator/Memory/WRamCgb.h"
#include "Emulator/Memory/EchoRam.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/HRam.h"
#include "Emulator/Memory/MBC/Mbc1.h"
#include "Emulator/Memory/AddressConstants.h"
#include <memory>

class Mbc1Test : public BaseTest
{
public:
    Mbc1Test();
    void Setup() override;
    void Run() override;

private:
    void TestBasicRomBankSwitching();
    void TestRamBankSwitching();
    void TestRamEnable();
    void TestBankingModes();
    void TestBankingModeEffects();
    void TestRomBankMasking();
    void TestAdvancedBankingSwitching();
    void TestBoundaryConditions();
    
    // Helper methods for assertions
    void VerifyMemoryValue(word address, byte expected, const char* description);
    void VerifyMemoryValueOneOf(word address, byte expected1, byte expected2, const char* description);

    std::unique_ptr<TestCpu> _cpu;
    std::unique_ptr<Bus> _bus;
    std::unique_ptr<BootRom> _bootRom;
    std::unique_ptr<Cartridge> _cartridge;
    std::unique_ptr<VRam> _vRam;
    std::unique_ptr<WRam> _wRam;
    std::unique_ptr<WRamCgb> _wRamCgb;
    std::unique_ptr<EchoRam> _echoRam;
    std::unique_ptr<Oam> _oam;
    std::unique_ptr<IoRegisters> _ioRegisters;
    std::unique_ptr<HRam> _hRam;
    
    std::vector<byte> _cartridgeData;
    std::vector<byte> _ramData;
};