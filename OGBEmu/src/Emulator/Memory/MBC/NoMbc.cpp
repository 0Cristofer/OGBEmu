#include "NoMbc.h"

#include <format>

#include "Core/Logger.h"
#include "Emulator/GbConstants.h"

#include "Emulator/Memory/AddressConstants.h"

NoMbc::NoMbc(std::vector<byte>* rom) : _rom(rom)
{
    const byte ramSizeFlag = (*_rom)[AddressConstants::CartridgeRamSizeAddress];
    
    if (ramSizeFlag == GbConstants::RamSizeFlag1Bank)
    {
        _ram = std::vector<byte>(GbConstants::RamBankSize);
        return;
    }
    
    if (ramSizeFlag != GbConstants::RamSizeFlagNoRam)
        DEBUGBREAKLOG("Invalid Cartridge RAM size: " << static_cast<int>((*_rom)[AddressConstants::CartridgeRamSizeAddress]) << ", defaulting to no ram");
}

NoMbc::~NoMbc()
{
    _rom = nullptr;
}

byte NoMbc::Read(word address)
{
    if (address >= _rom->size())
    {
        DEBUGBREAKLOG("Invalid NoMbc ROM read, address: " << std::format("{:x}", address));
        return 0;
    }

    return (*_rom)[address];
}

void NoMbc::Write(const word address, const byte data)
{
    const word translatedAddress = TranslateAddress(address);
    if (translatedAddress >= _ram.size())
    {
        DEBUGBREAKLOG("Invalid NoMbc RAM write, address: " << std::format("{:x}", address));
        return;
    }

    _ram[translatedAddress] = data;
}

word NoMbc::TranslateAddress(const word address)
{
    return address - AddressConstants::StartExternalRamAddress;
}