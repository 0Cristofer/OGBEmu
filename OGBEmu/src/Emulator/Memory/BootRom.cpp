#include "BootRom.h"

#include <format>

#include "Core/Logger.h"

BootRom::BootRom(std::vector<byte> romBytes): _rom(std::move(romBytes))
{
    if (!IsValid())
        return;
}

byte BootRom::Read(const word address) const
{
    if (address >= _rom.size())
    {
        DEBUGBREAKLOG("Invalid Boot ROM read, address: " << std::format("{:x}", address));
        return 0;
    }

    return _rom[address];
}
