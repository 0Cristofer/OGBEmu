#include "Mbc1.h"

#include <format>

#include "Core/Logger.h"

Mbc1::Mbc1(std::vector<byte>* rom) : _rom(rom)
{
}

byte Mbc1::Read(word address)
{
    DEBUGBREAKLOG("Mbc1 ROM read not implemented, address: " << std::format("{:x}", address));

    return 0;
}

void Mbc1::Write(word address, byte data)
{
    DEBUGBREAKLOG("Mbc1 ROM write not implemented, address: " << std::format("{:x}", address));
}
