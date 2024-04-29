#include "Device.h"

#include <memory>

Device::Device(BootRom bootRom, Cartridge cartridge, const int framesPerSecond) : _bootRom(std::move(bootRom)),
                                                                                  _cartridge(std::move(cartridge)), _ram(Ram()),
                                                                                  _bus(Bus(&_bootRom, &_cartridge, &_ram)),
                                                                                  _cpu(&_bus, framesPerSecond)
{
}

void Device::Run()
{
    _cpu.Run();
}
