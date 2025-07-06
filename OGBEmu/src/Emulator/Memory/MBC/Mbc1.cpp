#include "Mbc1.h"

#include <format>

#include "Core/Logger.h"
#include "Emulator/GbConstants.h"
#include "Emulator/Memory/AddressConstants.h"

Mbc1::Mbc1(std::vector<byte>* rom) : _rom(rom)
{
    const byte ramSizeFlag = (*_rom)[AddressConstants::CartridgeRamSizeAddress];

    byte numRamBanks;
    switch (ramSizeFlag)
    {
    case GbConstants::RamSizeFlagNoRam:
        numRamBanks = 0;
        break;    
    case GbConstants::RamSizeFlag1Bank:
        numRamBanks = 1;
        break;
    case GbConstants::RamSizeFlag4Bank:
        numRamBanks = 4;
        break;
    default:
        numRamBanks = 1;
        DEBUGBREAKLOG("Mbc1 invalid number of RAM banks, defaulting to " << numRamBanks);
        break;
    }

    const int ramSize = GbConstants::RamBankSize * numRamBanks;
    _ram = std::vector<byte>(ramSize);
    _bankSelect.ramBank = 0;
    _bankSelect.romBank = 0;
    _bankingMode = 0;
    _ramEnable = 0;
}

byte Mbc1::Read(word address)
{
    DEBUGBREAKLOG("Mbc1 ROM read not implemented, address: " << std::format("{:x}", address));

    if (address > AddressConstants::EndRomBank0Address)
    {
        if (address <= AddressConstants::EndRomBankNAddress)
        {
            byte romBank = 0;

            if (_bankSelect.ramBank == 0)
                romBank = 1;

            if (romBank > (*_rom)[AddressConstants::CartridgeRomSizeAddress])
        
                return 1;//(*_rom)[];   
        }

        // Read ram
    }

    return (*_rom)[address];
}

void Mbc1::Write(word address, byte data)
{
    DEBUGBREAKLOG("Mbc1 ROM write not implemented, address: " << std::format("{:x}", address));
}
