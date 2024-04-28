#pragma once

#include "Cartridge.h"
#include "Ram.h"
#include "Cpu.h"

class Device
{
public:
    explicit Device(Cartridge cartridge, const int framesPerSecond);

    void Run();

private:
    Cartridge _cartridge;
    Ram _ram;
    Bus _bus;
    Cpu _cpu;
};
