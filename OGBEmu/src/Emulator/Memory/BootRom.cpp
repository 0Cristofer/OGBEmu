#include "BootRom.h"

#include <format>

#include "Core/Logger.h"

BootRom::BootRom(const std::vector<byte>& romBytes): _rom(romBytes)
{
    if (!IsValid())
    {
        DEBUGBREAKLOG("Invalid boot ROM, check path and file size. Only " << GbConstants::BootRomSize <<"-byte ROMs are accepted.");
        return;
    }
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
