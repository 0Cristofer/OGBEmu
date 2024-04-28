#include "Device.h"

#include <chrono>

Device::Device(Cartridge cartridge, const int framesPerSecond) : _cartridge(std::move(cartridge)), _ram(Ram()),
                                                                 _bus(Bus(_cartridge, _ram)),
                                                                 _cpu(_bus, framesPerSecond)
{
}

void Device::Run()
{
    _cpu.Run();
}
