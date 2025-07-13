#include "Mbc3.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/GbConstants.h"
#include <format>

Mbc3::Mbc3(std::vector<byte>* rom) : _rom(rom), _romBank(1), _ramBank(0), _ramEnable(false), _latchClockData(0xFF)
{
    // Calculate number of ROM banks
    const byte romSizeFlag = (*_rom)[AddressConstants::CartridgeRomSizeAddress];
    _numRomBanks = 2 << romSizeFlag; // 2^(romSizeFlag + 1)
    
    // Calculate number of RAM banks
    const byte ramSizeFlag = (*_rom)[AddressConstants::CartridgeRamSizeAddress];
    switch (ramSizeFlag) {
        case 0: _numRamBanks = 0; break;  // No RAM
        case 1: _numRamBanks = 1; break;  // 2KB
        case 2: _numRamBanks = 1; break;  // 8KB (1 bank)
        case 3: _numRamBanks = 4; break;  // 32KB (4 banks of 8KB each)
        case 4: _numRamBanks = 16; break; // 128KB (16 banks)
        case 5: _numRamBanks = 8; break;  // 64KB (8 banks)
        default:
            ERROR("Invalid RAM size flag: " << static_cast<int>(ramSizeFlag));
            _numRamBanks = 0;
            break;
    }
    
    // Initialize RAM if present
    if (_numRamBanks > 0) {
        _ram.resize(_numRamBanks * GbConstants::RamBankSize, 0);
    }
    
    // Initialize RTC
    _rtc = {};
    _rtc.lastUpdate = std::chrono::steady_clock::now();
    _rtc.halted = false;
    
    LOG("MBC3 initialized with " << _numRomBanks << " ROM banks and " << _numRamBanks << " RAM banks");
}

byte Mbc3::Read(word address)
{
    if (address <= 0x3FFF) {
        // ROM Bank 0 (fixed)
        if (address < _rom->size()) {
            return (*_rom)[address];
        }
        return 0xFF;
    }
    else if (address <= 0x7FFF) {
        // Switchable ROM Bank (1-127)
        int bankOffset = GetRomBankOffset(_romBank);
        int localAddress = address - 0x4000;
        int romAddress = bankOffset + localAddress;
        
        if (romAddress < _rom->size()) {
            return (*_rom)[romAddress];
        }
        return 0xFF; // Return 0xFF for out-of-bounds reads
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        // RAM Bank or RTC Register
        if (!_ramEnable) {
            return 0xFF; // Return 0xFF when RAM/RTC disabled
        }
        
        if (IsRtcRegister(_ramBank)) {
            // Read from RTC register
            return ReadRtcRegister(_ramBank);
        }
        else if (IsValidRamBank(_ramBank) && !_ram.empty()) {
            // Read from RAM bank
            word bankOffset = GetRamBankOffset(_ramBank);
            int localAddress = address - 0xA000;
            int ramAddress = bankOffset + localAddress;
            
            if (ramAddress < _ram.size()) {
                return _ram[ramAddress];
            }
            return 0xFF;
        }
        
        return 0xFF;
    }
    
    ERROR("Invalid MBC3 ROM read, address: " << std::format("{:x}", address));
    return 0xFF;
}

void Mbc3::Write(word address, byte data)
{
    if (address <= 0x1FFF) {
        // RAM/RTC Enable (0x0000-0x1FFF)
        _ramEnable = (data & 0x0F) == 0x0A;
    }
    else if (address <= 0x3FFF) {
        // ROM Bank Number (0x2000-0x3FFF)
        byte bank = data & 0x7F; // 7-bit value
        if (bank == 0) {
            bank = 1; // Bank 0 maps to bank 1
        }
        
        if (IsValidRomBank(bank)) {
            _romBank = bank;
        }
        else {
            ERROR("Invalid MBC3 ROM bank: " << static_cast<int>(bank));
        }
    }
    else if (address <= 0x5FFF) {
        // RAM Bank Number or RTC Register Select (0x4000-0x5FFF)
        _ramBank = data;
    }
    else if (address <= 0x7FFF) {
        // Latch Clock Data (0x6000-0x7FFF)
        if (_latchClockData == 0x00 && data == 0x01) {
            // Latch clock data on 0x00 -> 0x01 transition
            LatchClockData();
        }
        _latchClockData = data;
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        // RAM Bank or RTC Register
        if (!_ramEnable) {
            return; // Ignore writes when RAM/RTC disabled
        }
        
        if (IsRtcRegister(_ramBank)) {
            // Write to RTC register
            WriteRtcRegister(_ramBank, data);
        }
        else if (IsValidRamBank(_ramBank) && !_ram.empty()) {
            // Write to RAM bank
            word bankOffset = GetRamBankOffset(_ramBank);
            int localAddress = address - 0xA000;
            int ramAddress = bankOffset + localAddress;
            
            if (ramAddress < _ram.size()) {
                _ram[ramAddress] = data;
            }
        }
    }
    else {
        ERROR("Invalid MBC3 RAM write, address: " << std::format("{:x}", address));
    }
}

int Mbc3::GetRomBankOffset(byte bank) const
{
    return bank * GbConstants::RomBankSize;
}

word Mbc3::GetRamBankOffset(byte bank) const
{
    return bank * GbConstants::RamBankSize;
}

bool Mbc3::IsValidRomBank(byte bank) const
{
    return bank > 0 && bank < _numRomBanks;
}

bool Mbc3::IsValidRamBank(byte bank) const
{
    return bank < _numRamBanks;
}

bool Mbc3::IsRtcRegister(byte bank) const
{
    return bank >= 0x08 && bank <= 0x0C;
}

void Mbc3::UpdateRtc()
{
    if (_rtc.halted) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - _rtc.lastUpdate);
    _rtc.lastUpdate = now;
    
    if (elapsed.count() == 0) {
        return;
    }
    
    // Add elapsed seconds
    int totalSeconds = _rtc.seconds + static_cast<int>(elapsed.count());
    
    // Handle seconds overflow
    if (totalSeconds >= 60) {
        int minutes = totalSeconds / 60;
        _rtc.seconds = totalSeconds % 60;
        
        // Add to minutes
        int totalMinutes = _rtc.minutes + minutes;
        if (totalMinutes >= 60) {
            int hours = totalMinutes / 60;
            _rtc.minutes = totalMinutes % 60;
            
            // Add to hours
            int totalHours = _rtc.hours + hours;
            if (totalHours >= 24) {
                int days = totalHours / 24;
                _rtc.hours = totalHours % 24;
                
                // Add to days
                int totalDays = (_rtc.dayHigh & 0x01) << 8 | _rtc.dayLow;
                totalDays += days;
                
                // Handle day overflow (9-bit counter, max 511 days)
                if (totalDays > 511) {
                    totalDays = totalDays % 512;
                    _rtc.dayHigh |= 0x80; // Set day carry flag
                }
                
                _rtc.dayLow = totalDays & 0xFF;
                _rtc.dayHigh = (_rtc.dayHigh & 0xFE) | ((totalDays >> 8) & 0x01);
            }
        }
    }
    else {
        _rtc.seconds = totalSeconds;
    }
}

void Mbc3::LatchClockData()
{
    UpdateRtc();
    
    _rtc.latchedSeconds = _rtc.seconds;
    _rtc.latchedMinutes = _rtc.minutes;
    _rtc.latchedHours = _rtc.hours;
    _rtc.latchedDayLow = _rtc.dayLow;
    _rtc.latchedDayHigh = _rtc.dayHigh;
}

byte Mbc3::ReadRtcRegister(byte reg) const
{
    switch (reg) {
        case 0x08: return _rtc.latchedSeconds;  // RTC_S
        case 0x09: return _rtc.latchedMinutes;  // RTC_M
        case 0x0A: return _rtc.latchedHours;    // RTC_H
        case 0x0B: return _rtc.latchedDayLow;   // RTC_DL
        case 0x0C: return _rtc.latchedDayHigh;  // RTC_DH
        default:
            ERROR("Invalid RTC register read: " << std::format("{:x}", reg));
            return 0xFF;
    }
}

void Mbc3::WriteRtcRegister(byte reg, byte data)
{
    switch (reg) {
        case 0x08: // RTC_S
            _rtc.seconds = data % 60;
            break;
        case 0x09: // RTC_M
            _rtc.minutes = data % 60;
            break;
        case 0x0A: // RTC_H
            _rtc.hours = data % 24;
            break;
        case 0x0B: // RTC_DL
            _rtc.dayLow = data;
            break;
        case 0x0C: // RTC_DH
            _rtc.dayHigh = data;
            _rtc.halted = (data & 0x40) != 0; // Bit 6 is halt flag
            if (!_rtc.halted) {
                _rtc.lastUpdate = std::chrono::steady_clock::now();
            }
            break;
        default:
            ERROR("Invalid RTC register write: " << std::format("{:x}", reg));
            break;
    }
}