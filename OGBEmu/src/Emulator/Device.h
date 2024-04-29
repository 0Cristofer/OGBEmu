#pragma once

#include "BootRom.h"
#include "Cartridge.h"
#include "Ram.h"
#include "Bus.h"
#include "Cpu.h"

class Device
{
public:
    explicit Device(BootRom bootRom, Cartridge cartridge, int framesPerSecond);

    void Run();

private:
    BootRom _bootRom;
    Cartridge _cartridge;
    Ram _ram;
    Bus _bus;
    Cpu _cpu;
};
