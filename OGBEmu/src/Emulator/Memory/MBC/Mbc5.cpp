#include "Mbc5.h"

#include <format>

#include "Core/Logger.h"
#include "Emulator/GbConstants.h"
#include "Emulator/Memory/AddressConstants.h"

Mbc5::Mbc5(std::vector<byte>* rom) : _rom(rom), _romBank(1), _ramBank(0), _ramEnable(false)
{
    // Calculate number of ROM banks
    const byte romSizeFlag = (*_rom)[AddressConstants::CartridgeRomSizeAddress];
    _numRomBanks = 2 << romSizeFlag; // 2^(romSizeFlag + 1)
    
    // Calculate number of RAM banks
    const byte ramSizeFlag = (*_rom)[AddressConstants::CartridgeRamSizeAddress];
    switch (ramSizeFlag)
    {
    case GbConstants::RamSizeFlagNoRam:
        _numRamBanks = 0;
        break;
    case GbConstants::RamSizeFlag1Bank:
        _numRamBanks = 1;
        break;
    case GbConstants::RamSizeFlag4Bank:
        _numRamBanks = 4;
        break;
    case GbConstants::RamSizeFlag8Bank:
        _numRamBanks = 8;
        break;
    case GbConstants::RamSizeFlag16Bank:
        _numRamBanks = 16;
        break;
    default:
        _numRamBanks = 0;
        ERROR("MBC5 invalid RAM size flag: " << std::format("{:x}", ramSizeFlag));
        break;
    }
    
    // Initialize RAM
    if (_numRamBanks > 0)
    {
        const int ramSize = GbConstants::RamBankSize * _numRamBanks;
        _ram = std::vector<byte>(ramSize, 0);
    }
    
    LOG("MBC5 initialized with " << _numRomBanks << " ROM banks and " << _numRamBanks << " RAM banks");
}

byte Mbc5::Read(word address)
{
    // ROM Bank 0 (0x0000-0x3FFF) - always mapped to bank 0
    if (address <= AddressConstants::EndRomBank0Address)
    {
        if (address < _rom->size())
        {
            return (*_rom)[address];
        }
        return 0xFF;
    }
    
    // ROM Bank N (0x4000-0x7FFF) - switchable bank
    if (address >= AddressConstants::StartRomBankNAddress && address <= AddressConstants::EndRomBankNAddress)
    {
        const int bankOffset = GetRomBankOffset(_romBank);
        const int romAddress = bankOffset + (address - AddressConstants::StartRomBankNAddress);
        
        if (romAddress < _rom->size())
        {
            return (*_rom)[romAddress];
        }
        return 0xFF;
    }
    
    // External RAM (0xA000-0xBFFF) - switchable bank
    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
    {
        if (!_ramEnable || _numRamBanks == 0)
        {
            return 0xFF;
        }
        
        if (!IsValidRamBank(_ramBank))
        {
            return 0xFF;
        }
        
        const word ramOffset = GetRamBankOffset(_ramBank);
        const word ramAddress = ramOffset + (address - AddressConstants::StartExternalRamAddress);
        
        if (ramAddress < _ram.size())
        {
            return _ram[ramAddress];
        }
        return 0xFF;
    }
    
    ERROR("MBC5 Read called with invalid address: " << std::format("{:x}", address));
    return 0xFF;
}

void Mbc5::Write(word address, byte data)
{
    // RAM Enable (0x0000-0x1FFF)
    if (address <= 0x1FFF)
    {
        _ramEnable = (data & 0x0F) == 0x0A;
        return;
    }
    
    // ROM Bank Number Low 8 bits (0x2000-0x2FFF)
    if (address >= 0x2000 && address <= 0x2FFF)
    {
        _romBank = (_romBank & 0x100) | data;
        
        // Ensure valid bank
        if (!IsValidRomBank(_romBank))
        {
            _romBank = _romBank % _numRomBanks;
        }
        return;
    }
    
    // ROM Bank Number High 1 bit (0x3000-0x3FFF)
    if (address >= 0x3000 && address <= 0x3FFF)
    {
        _romBank = (_romBank & 0xFF) | ((data & 0x01) << 8);
        
        // Ensure valid bank
        if (!IsValidRomBank(_romBank))
        {
            _romBank = _romBank % _numRomBanks;
        }
        return;
    }
    
    // RAM Bank Number (0x4000-0x5FFF)
    if (address >= 0x4000 && address <= 0x5FFF)
    {
        _ramBank = data & 0x0F; // Only 4 bits used
        
        // Ensure valid bank
        if (!IsValidRamBank(_ramBank))
        {
            _ramBank = _ramBank % (_numRamBanks > 0 ? _numRamBanks : 1);
        }
        return;
    }
    
    // External RAM Write (0xA000-0xBFFF)
    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
    {
        if (!_ramEnable || _numRamBanks == 0)
        {
            return;
        }
        
        if (!IsValidRamBank(_ramBank))
        {
            return;
        }
        
        const word ramOffset = GetRamBankOffset(_ramBank);
        const word ramAddress = ramOffset + (address - AddressConstants::StartExternalRamAddress);
        
        if (ramAddress < _ram.size())
        {
            _ram[ramAddress] = data;
        }
        return;
    }
}

int Mbc5::GetRomBankOffset(word bank) const
{
    return bank * GbConstants::RomBankSize;
}

word Mbc5::GetRamBankOffset(byte bank) const
{
    return bank * GbConstants::RamBankSize;
}

bool Mbc5::IsValidRomBank(word bank) const
{
    return bank < _numRomBanks;
}

bool Mbc5::IsValidRamBank(byte bank) const
{
    return bank < _numRamBanks;
}