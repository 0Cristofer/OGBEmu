#pragma once

#include <string>
#include <vector>

#include "Core/Definitions.h"

#include "Emulator/GbConstants.h"

class BaseMbc;

enum class CartridgeType : byte
{
    RomOnly = 0x00,
    MBC1 = 0x01,
    MBC1Ram = 0x02,
    MBC1RamBattery = 0x03,
    MBC2 = 0x05,
    MBC2Battery = 0x06,
    RomRam = 0x08,
    RomRamBattery = 0x09,
    MMM01 = 0x0B,
    MMM01Ram = 0x0C,
    MMM01RamBattery = 0x0D,
    MBC3TimerBattery = 0x0F,
    MBC3TimerRamBattery = 0x10,
    MBC3 = 0x11,
    MBC3Ram = 0x12,
    MBC3RamBattery = 0x13,
    MBC5 = 0x19,
    MBC5Ram = 0x1A,
    MBC5RamBattery = 0x1B,
    MBC5Rumble = 0x1C,
    MBC5RumbleRam = 0x1D,
    MBC5RumbleRamBattery = 0x1E,
    MBC6 = 0x20,
    MBC7SensorRumbleRamBattery = 0x22,
    PocketCamera = 0xFC,
    BandaiTama5 = 0xFD,
    HuC3 = 0xFE,
    HuC1RamBattery = 0xFF,
};

class Cartridge
{
public:
    explicit Cartridge(const std::vector<byte>& romBytes);
    ~Cartridge();

    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] byte Read(word address) const;
    void Write(word address, byte data) const;

private:
    [[nodiscard]] std::string GetStringFromHeader(word startAddress, word endAddress) const;
    
    std::vector<byte> _rom;
    BaseMbc* _mbc = nullptr;
    
    CartridgeType _cartridgeType;
    std::string _title;
    std::string _manufacturerCode;
    byte _oldLicenseeCode = 0;
    std::string _newLicenseeCode;
};
