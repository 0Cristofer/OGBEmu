#pragma once

#include "BaseTest.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/Memory/WRam.h"
#include "Emulator/Memory/WRamCgb.h"
#include "Emulator/Memory/EchoRam.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/HRam.h"
#include "TestCpu.h"
#include <memory>

class VRamTest : public BaseTest
{
public:
    VRamTest();
    
    void Setup() override;
    void Run() override;

private:
    void TestVRamBasicReadWrite();
    void TestVRamAddressTranslation();
    void TestVRamBoundaryConditions();
    void TestVRamPatternStorage();
    void TestVRamTileDataAccess();
    void TestVRamBankingSwitching();
    void TestVRamConstants();
    
    void VerifyVRamValue(word address, byte expected, const std::string& testName);
    void WriteProgramToCartridge(const std::vector<byte>& program);
    void ExecuteInstructions(int count);
    
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
    
    std::vector<byte> _cartridgeData;
    std::vector<byte> _bootRomData;
};