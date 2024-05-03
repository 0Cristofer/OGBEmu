#include "BootRom.h"

#include "Core/Logger.h"

BootRom::BootRom(std::vector<byte> romBytes): _rom(std::move(romBytes))
{
    if (!IsValid())
    {
        return;
    }
}

byte& BootRom::ReadRef(const word address)
{
    if (address >= _rom.size())
    {
        LOG("Invalid Boot ROM read, address: " << address);
        return ReadRef(0);
    }

    return _rom[address];
}

byte BootRom::Read(const word address)
{
    return ReadRef(address);
}
