#pragma once

#include <memory>
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
#include "Emulator/IScreen.h"
#include "Emulator/Ppu.h"
#include "Emulator/Timer.h"
#include "Emulator/Joypad.h"
#include "Emulator/Dma.h"
#include "Emulator/Apu.h"

class Device
{
public:
    Device(const std::vector<byte>& bootRomBytes, const std::vector<byte>& cartridgeBytes, int framesPerSecond, double timeoutSeconds, IScreen* screen);

    [[nodiscard]] bool IsValid() const;
    void Run();

private:
    unsigned long DoFrame();
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
    IScreen* _screen;
    Ppu _ppu;
    Timer _timer;
    Joypad _joypad;
    Dma _dma;
    Apu _apu;

    unsigned int _framesPerSecond;
    double _frameTimeSeconds;
    double _maxCyclesPerFrame;
    double _timeoutSeconds;
};
