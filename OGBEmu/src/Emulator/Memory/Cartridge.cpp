#include "Cartridge.h"

#include <format>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/MBC/BaseMbc.h"
#include "Emulator/Memory/MBC/Mbc1.h"
#include "Emulator/Memory/MBC/Mbc3.h"
#include "Emulator/Memory/MBC/Mbc5.h"
#include "Emulator/Memory/MBC/NoMbc.h"

Cartridge::Cartridge(const std::vector<byte>& romBytes) : _rom(romBytes), _mbc(nullptr)
{
    if (!IsValid())
    {
        ERROR("Invalid cartridge ROM, check path and file size. Expected ROM size: " << std::format("{:x}", static_cast<int>(GbConstants::MinCartridgeRomSize) * (1 << _rom[AddressConstants::CartridgeRomSizeAddress]))
            << ", got: " << std::format("{:x}", _rom.size()));
        return;
    }

    _cartridgeType = static_cast<CartridgeType>(_rom[AddressConstants::CartridgeTypeAddress]);

    const bool isCgb = _rom[AddressConstants::CartridgeCgbFlagAddress] == GbConstants::CgbFlag;
    const word titleEndAddress = isCgb ? AddressConstants::CartridgeTitleNewEndAddress : AddressConstants::CartridgeTitleOldEndAddress;
    
    _title = GetStringFromHeader(AddressConstants::CartridgeTitleStartAddress, titleEndAddress);

    if (isCgb)
    {
        _manufacturerCode = GetStringFromHeader(AddressConstants::CartridgeManufacturerCodeStartAddress, AddressConstants::CartridgeManufacturerCodeEndAddress);
    }
    else
        _manufacturerCode = "";

    if (_rom[AddressConstants::CartridgeOldLicenseeCodeAddress] == GbConstants::NewLicenseeCode)
    {
        _newLicenseeCode = GetStringFromHeader(AddressConstants::CartridgeNewLicenseeCodeStartAddress, AddressConstants::CartridgeNewLicenseeCodeEndAddress);
        _oldLicenseeCode = 0;
    }
    else
    {
        _oldLicenseeCode = _rom[AddressConstants::CartridgeOldLicenseeCodeAddress];
        _newLicenseeCode = "";
    }

    switch (_cartridgeType)
    {
    case CartridgeType::RomOnly:
        _mbc = new NoMbc(&_rom);
        return;
    case CartridgeType::MBC1:
    case CartridgeType::MBC1Ram:
    case CartridgeType::MBC1RamBattery:
        _mbc = new Mbc1(&_rom);
        return;
    case CartridgeType::MBC2:
    case CartridgeType::MBC2Battery:
    case CartridgeType::RomRam:
    case CartridgeType::RomRamBattery:
    case CartridgeType::MMM01:
    case CartridgeType::MMM01Ram:
    case CartridgeType::MMM01RamBattery:
    case CartridgeType::MBC3TimerBattery:
    case CartridgeType::MBC3TimerRamBattery:
    case CartridgeType::MBC3:
    case CartridgeType::MBC3Ram:
    case CartridgeType::MBC3RamBattery:
        _mbc = new Mbc3(&_rom);
        return;
    case CartridgeType::MBC5:
    case CartridgeType::MBC5Ram:
    case CartridgeType::MBC5RamBattery:
    case CartridgeType::MBC5Rumble:
    case CartridgeType::MBC5RumbleRam:
    case CartridgeType::MBC5RumbleRamBattery:
        _mbc = new Mbc5(&_rom);
        return;
    case CartridgeType::MBC6:
    case CartridgeType::MBC7SensorRumbleRamBattery:
    case CartridgeType::PocketCamera:
    case CartridgeType::BandaiTama5:
    case CartridgeType::HuC3:
    case CartridgeType::HuC1RamBattery:
        ERROR("Cartridge type not implemented, cartridge type: " << std::format("{:x}", static_cast<int>(_cartridgeType)));
        ERROR("MBC will be null - this will cause crashes on cartridge access");
        break;
    default:
        ERROR("Unknown cartridge type: " << std::format("{:x}", static_cast<int>(_cartridgeType)));
        ERROR("MBC will be null - this will cause crashes on cartridge access");
        break;
    }
}

Cartridge::~Cartridge()
{
    delete _mbc;
}

bool Cartridge::IsValid() const
{
    if (_rom.empty() || _rom.size() <= AddressConstants::CartridgeRamSizeAddress) {
        return false;
    }
    return static_cast<int>(_rom.size()) == GbConstants::MinCartridgeRomSize * (1 << _rom[AddressConstants::CartridgeRomSizeAddress]);
}

byte Cartridge::Read(const word address) const
{
    if (_mbc == nullptr) {
        ERROR("Cartridge MBC is null during read at address " << std::format("{:x}", address));
        return 0x00;
    }
    return _mbc->Read(address);
}

void Cartridge::Write(const word address, const byte data) const
{
    if (_mbc == nullptr) {
        ERROR("Cartridge MBC is null during write at address " << std::format("{:x}", address) << " with data " << std::format("{:x}", data));
        return;
    }
    _mbc->Write(address, data);
}

std::string Cartridge::GetStringFromHeader(const word startAddress, const word endAddress) const
{
    std::stringstream stringStream;
    for (int i = startAddress; i <= endAddress; i++)
    {
        stringStream << _rom[i];

        if (_rom[i] == '\0')
            break;
    }
    stringStream << '\0';

    return stringStream.str();
}

