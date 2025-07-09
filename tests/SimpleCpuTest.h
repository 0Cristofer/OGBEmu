#pragma once

#include "BaseTest.h"
#include "Core/Definitions.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/Memory/EchoRam.h"
#include "Emulator/Memory/HRam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/WRam.h"
#include "Emulator/Memory/WRamCgb.h"
#include "Emulator/Cpu.h"
#include <memory>

class SimpleCpuTest : public BaseTest
{
public:
    SimpleCpuTest();

    void Setup() override;
    void Run() override;

private:
    // Memory components
    std::vector<byte> _bootRomData;
    std::vector<byte> _cartridgeData;
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
    std::unique_ptr<Cpu> _cpu;
};