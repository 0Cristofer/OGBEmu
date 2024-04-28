#pragma once


class BootRom;
class Cartridge;
class Ram;

class Bus
{
public:
    Bus(BootRom& bootRom, Cartridge& cartridge, Ram& ram);

private:
    BootRom& _bootRom;
    Cartridge& _cartridge;
    Ram& _ram;
};
