#pragma once

#include "BaseTest.h"
#include <vector>
#include <memory>

#include "../OGBEmu/src/Emulator/Memory/Bus.h"
#include "../OGBEmu/src/Emulator/Memory/BootRom.h"
#include "../OGBEmu/src/Emulator/Memory/Cartridge.h"
#include "../OGBEmu/src/Emulator/Memory/VRam.h"
#include "../OGBEmu/src/Emulator/Memory/WRam.h"
#include "../OGBEmu/src/Emulator/Memory/WRamCgb.h"
#include "../OGBEmu/src/Emulator/Memory/EchoRam.h"
#include "../OGBEmu/src/Emulator/Memory/Oam.h"
#include "../OGBEmu/src/Emulator/Memory/IoRegisters.h"
#include "../OGBEmu/src/Emulator/Memory/HRam.h"
#include "../OGBEmu/src/Emulator/Memory/AddressConstants.h"

class BootRomDisableTest : public BaseTest
{
public:
    BootRomDisableTest() : BaseTest("Boot ROM Disable Mechanism") {}

    void Setup() override;
    void Run() override;

private:
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
};