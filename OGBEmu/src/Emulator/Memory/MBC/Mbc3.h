#pragma once

#include <vector>
#include <chrono>

#include "BaseMbc.h"

class Mbc3 : public BaseMbc
{
public:
    explicit Mbc3(std::vector<byte>* rom);
    
    byte Read(word address) override;
    void Write(word address, byte data) override;

private:
    std::vector<byte>* _rom;
    std::vector<byte> _ram;
    
    // MBC3 banking registers
    byte _romBank;      // 7-bit ROM bank number (1-127, bank 0 fixed)
    byte _ramBank;      // 2-bit RAM bank number (0-3) or RTC register select
    bool _ramEnable;    // RAM/RTC enable flag
    
    // ROM/RAM size information
    int _numRomBanks;
    int _numRamBanks;
    
    // Real-Time Clock (RTC) support
    struct RtcData {
        byte seconds;       // RTC_S (0-59)
        byte minutes;       // RTC_M (0-59) 
        byte hours;         // RTC_H (0-23)
        byte dayLow;        // RTC_DL (lower 8 bits of day counter)
        byte dayHigh;       // RTC_DH (upper 1 bit of day, halt flag, day carry flag)
        
        // Latched values
        byte latchedSeconds;
        byte latchedMinutes;
        byte latchedHours;
        byte latchedDayLow;
        byte latchedDayHigh;
        
        std::chrono::steady_clock::time_point lastUpdate;
        bool halted;
    } _rtc;
    
    byte _latchClockData;   // Clock data latch register
    
    // Helper methods
    int GetRomBankOffset(byte bank) const;
    word GetRamBankOffset(byte bank) const;
    bool IsValidRomBank(byte bank) const;
    bool IsValidRamBank(byte bank) const;
    bool IsRtcRegister(byte bank) const;
    void UpdateRtc();
    void LatchClockData();
    byte ReadRtcRegister(byte reg) const;
    void WriteRtcRegister(byte reg, byte data);
};