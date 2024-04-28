#include "Bus.h"

Bus::Bus(BootRom& bootRom, Cartridge& cartridge, Ram& ram) : _bootRom(bootRom), _cartridge(cartridge), _ram(ram)
{
}
