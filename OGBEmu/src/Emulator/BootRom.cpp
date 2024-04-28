#include "BootRom.h"

#include "Core/Logger.h"

BootRom::BootRom(std::vector<byte> romBytes): _rom(std::move(romBytes))
{
    if (!IsValid())
    {
        return;
    }
}

byte BootRom::Read(const word address) const
{
    if (address > _rom.size())
    {
        LOG("Invalid Boot ROM read, address: " << address);
        return 0;
    }

    return _rom[address];
}
