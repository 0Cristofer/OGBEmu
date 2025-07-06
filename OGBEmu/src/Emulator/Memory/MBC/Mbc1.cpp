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
    _bankSelect.romBank = 1;
    _bankingMode = 0;
    _ramEnable = 0;
}

byte Mbc1::Read(word address)
{
    // Only handle addresses that MBC1 should respond to
    if (address > AddressConstants::EndExternalRamAddress)
    {
        DEBUGBREAKLOG("MBC1 Read called with invalid address: " << std::format("{:x}", address));
        return 0xFF;
    }
    
    // ROM Bank 0 (0x0000-0x3FFF)
    if (address <= AddressConstants::EndRomBank0Address)
    {
        // In banking mode 1, ROM bank 0 is affected by upper bits for large ROMs
        if (_bankingMode == 1)
        {
            byte romBank = (_bankSelect.ramBank << 5);
            const word bankOffset = address + (romBank * GbConstants::RomBankSize);
            if (bankOffset < _rom->size())
            {
                return (*_rom)[bankOffset];
            }
        }
        return (*_rom)[address];
    }
    
    // ROM Bank 1-127 (0x4000-0x7FFF)
    if (address <= AddressConstants::EndRomBankNAddress)
    {
        byte romBank = _bankSelect.romBank;
        if (romBank == 0) romBank = 1; // Bank 0 maps to bank 1
        
        // In banking mode 1, use RAM bank as upper bits
        if (_bankingMode == 1)
        {
            romBank |= (_bankSelect.ramBank << 5);
        }
        
        const word bankOffset = (address - AddressConstants::StartRomBankNAddress) + (romBank * GbConstants::RomBankSize);
        if (bankOffset < _rom->size())
        {
            const byte data = (*_rom)[bankOffset];
            
            // Log ROM bank reads that return test pattern values
            if (data == 0x39 || data == 0x00)
            {
                DEBUGBREAKLOG("MBC1 ROM Bank " << static_cast<int>(romBank) << " read: addr=" << std::format("{:x}", address) << 
                             " bank_addr=" << std::format("{:x}", bankOffset) << " data=" << std::format("{:x}", data));
            }
            
            return data;
        }
        return 0xFF;
    }
    
    // External RAM (0xA000-0xBFFF)
    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
    {
        if (_ramEnable != 0x0A || _ram.empty())
        {
            return 0xFF;
        }
        
        byte ramBank = (_bankingMode == 1) ? _bankSelect.ramBank : 0;
        const word ramOffset = (address - AddressConstants::StartExternalRamAddress) + (ramBank * GbConstants::RamBankSize);
        
        if (ramOffset < _ram.size())
        {
            return _ram[ramOffset];
        }
        return 0xFF;
    }
    
    return 0xFF;
}

void Mbc1::Write(word address, byte data)
{
    // Only handle addresses that MBC1 should respond to
    if (address > AddressConstants::EndExternalRamAddress)
    {
        DEBUGBREAKLOG("MBC1 Write called with invalid address: " << std::format("{:x}", address));
        return;
    }
    
    // RAM Enable (0x0000-0x1FFF)
    if (address <= 0x1FFF)
    {
        _ramEnable = (data & 0x0F) == 0x0A ? 0x0A : 0x00;
        return;
    }
    
    // ROM Bank Number (0x2000-0x3FFF)
    if (address >= 0x2000 && address <= 0x3FFF)
    {
        const byte oldBank = _bankSelect.romBank;
        _bankSelect.romBank = data & 0x1F; // 5 bits
        if (_bankSelect.romBank == 0)
        {
            _bankSelect.romBank = 1;
        }
        
        // Log bank switches for debugging
        if (oldBank != _bankSelect.romBank)
        {
            DEBUGBREAKLOG("MBC1 ROM bank switch: " << static_cast<int>(oldBank) << " -> " << static_cast<int>(_bankSelect.romBank) << 
                         " (write " << std::format("{:x}", data) << " to " << std::format("{:x}", address) << ")");
        }
        
        return;
    }
    
    // RAM Bank Number / Upper Bits of ROM Bank Number (0x4000-0x5FFF)
    if (address >= 0x4000 && address <= 0x5FFF)
    {
        _bankSelect.ramBank = data & 0x03; // 2 bits
        return;
    }
    
    // Banking Mode Select (0x6000-0x7FFF)
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        _bankingMode = data & 0x01;
        return;
    }
    
    // External RAM (0xA000-0xBFFF)
    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
    {
        if (_ramEnable != 0x0A || _ram.empty())
        {
            return;
        }
        
        byte ramBank = (_bankingMode == 1) ? _bankSelect.ramBank : 0;
        const word ramOffset = (address - AddressConstants::StartExternalRamAddress) + (ramBank * GbConstants::RamBankSize);
        
        if (ramOffset < _ram.size())
        {
            _ram[ramOffset] = data;
        }
    }
}
