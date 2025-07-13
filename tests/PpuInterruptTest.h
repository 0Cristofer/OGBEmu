#pragma once

#include <memory>
#include "BaseTest.h"
#include "TestCpu.h"
#include "MockScreen.h"
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
#include "Emulator/Ppu.h"

class PpuInterruptTest : public BaseTest
{
public:
    PpuInterruptTest();

protected:
    void Setup() override;
    void Run() override;

private:
    void TestVBlankInterruptTriggering();
    void TestStatRegisterModeTracking();
    void TestStatInterruptGeneration();
    void TestLyLycComparison();
    void TestPpuModeStateTransitions();
    void TestHBlankInterrupt();
    void TestOamInterrupt();
    void TestLyLycInterrupt();
    
    void WriteProgramToCartridge(const std::vector<byte>& program);
    void ExecuteCycles(int cycles);
    void WaitForScanline(byte targetScanline);
    void EnableStatInterrupt(byte interruptBits);
    
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
    std::unique_ptr<MockScreen> _screen;
    std::unique_ptr<Ppu> _ppu;
    
    std::vector<byte> _bootRomData;
    std::vector<byte> _cartridgeData;
};