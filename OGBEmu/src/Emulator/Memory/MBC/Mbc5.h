#pragma once

#include <vector>

#include "BaseMbc.h"

class Mbc5 : public BaseMbc
{
public:
    explicit Mbc5(std::vector<byte>* rom);
    
    byte Read(word address) override;
    void Write(word address, byte data) override;

private:
    std::vector<byte>* _rom;
    std::vector<byte> _ram;
    
    // MBC5 banking registers
    word _romBank;      // 9-bit ROM bank number (0-511)
    byte _ramBank;      // 4-bit RAM bank number (0-15)
    bool _ramEnable;    // RAM enable flag
    
    // ROM/RAM size information
    int _numRomBanks;
    int _numRamBanks;
    
    // Helper methods
    int GetRomBankOffset(word bank) const;
    word GetRamBankOffset(byte bank) const;
    bool IsValidRomBank(word bank) const;
    bool IsValidRamBank(byte bank) const;
};