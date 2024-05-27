#include "Cartridge.h"

#include <format>

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

byte Cartridge::Read(const word address) const
{
    if (address >= _rom.size())
    {
        DEBUGBREAKLOG("Invalid Cartridge ROM read, address: " << std::format("{:x}", address));
        return 0;
    }

    return _rom[address];
}

void Cartridge::Write(word address, byte data)
{
    DEBUGBREAKLOG("Cartridge write not implemented, address: " << std::format("{:x}", address));
}

