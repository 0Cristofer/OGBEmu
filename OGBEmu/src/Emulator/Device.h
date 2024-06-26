#pragma once

#include "Emulator/Cpu.h"
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

class Device
{
public:
    Device(const std::vector<byte>& bootRomBytes, const std::vector<byte>& cartridgeBytes, int framesPerSecond);

    [[nodiscard]] bool IsValid() const;
    void Run();

private:
    unsigned int DoFrame();
    void WaitForNextFrame(double frameTimeSeconds) const;

    BootRom _bootRom;
    Cartridge _cartridge;
    VRam _vRam;
    WRam _wRam;
    WRamCgb _wRamCgb;
    EchoRam _echoRam;
    Oam _oam;
    IoRegisters _ioRegisters;
    HRam _hRam;
    Bus _bus;
    Cpu _cpu;

    unsigned int _framesPerSecond;
    double _frameTimeSeconds;
    double _maxCyclesPerFrame;
};
