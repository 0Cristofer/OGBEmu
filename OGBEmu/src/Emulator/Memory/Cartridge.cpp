#include "Cartridge.h"

#include <utility>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

Cartridge::Cartridge(std::vector<byte> romBytes) : _rom(std::move(romBytes))
{
    if (!IsValid())
    {
        return;
    }

    _cartridgeType = static_cast<CartridgeType>(_rom[AddressConstants::CartridgeTypeAddress]);
}

byte Cartridge::Read(const word address)
{
    if (address >= _rom.size())
    {
        LOG("Invalid Cartridge ROM read, address: " << address);
        return Read(0);
    }

    return _rom[address];
}

