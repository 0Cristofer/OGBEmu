#include "Cartridge.h"

#include <utility>

#include "Core/Logger.h"

Cartridge::Cartridge(std::vector<byte> romBytes) : _rom(std::move(romBytes))
{
    if (!IsValid())
    {
        return;
    }

    _cartridgeType = static_cast<CartridgeType>(_rom[GbConstants::CartridgeTypeAddress]);
}

byte Cartridge::Read(const word address) const
{
    if (address >= _rom.size())
    {
        LOG("Invalid Cartridge ROM read, address: " << address);
        return 0;
    }

    return _rom[address];
}
