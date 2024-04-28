#include "BootRom.h"

BootRom::BootRom(std::vector<unsigned char> romBytes): _rom(std::move(romBytes))
{
    if (!IsValid())
    {
        return;
    }
}
