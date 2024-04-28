#pragma once
#include "Cartridge.h"
#include "Ram.h"

class Bus
{
public:
    Bus(Cartridge& cartridge, Ram& ram);

private:
    Cartridge& _cartridge;
    Ram& _ram;
};
