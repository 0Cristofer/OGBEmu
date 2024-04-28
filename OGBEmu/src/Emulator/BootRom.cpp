#include "BootRom.h"

BootRom::BootRom(std::vector<byte> romBytes): _rom(std::move(romBytes))
{
    if (!IsValid())
    {
        return;
    }
}
