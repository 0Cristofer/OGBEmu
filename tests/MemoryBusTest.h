#pragma once

#include "BaseTest.h"
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
#include <memory>

class MemoryBusTest : public BaseTest
{
public:
    MemoryBusTest();

    void Setup() override;
    void Run() override;

private:
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

    // Test data
    std::vector<byte> _bootRomData;
    std::vector<byte> _cartridgeData;

    // Test methods
    void TestBootRomRouting();
    void TestCartridgeRouting();
    void TestVRamRouting();
    void TestWRamRouting();
    void TestWRamCgbRouting();
    void TestEchoRamRouting();
    void TestOamRouting();
    void TestIoRegistersRouting();
    void TestHRamRouting();
    void TestIeAddressRouting();
    void TestNotUsedAddressRange();
    void TestAddressBoundaries();

    // Helper methods
    void VerifyReadWrite(word address, byte testValue, const std::string& regionName);
    void VerifyReadOnly(word address, const std::string& regionName);
};