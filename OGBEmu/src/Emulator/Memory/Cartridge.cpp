#include "Cartridge.h"

#include <format>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

Cartridge::Cartridge(std::vector<byte> romBytes) : _rom(std::move(romBytes))
{
    if (!IsValid())
    {
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
        _newLicenseeCode = GetStringFromHeader(AddressConstants::CartridgeNewLicenseeCodeStartAddress, AddressConstants::CartridgeNewLicenseeCodeEndAddress);
    else
        _oldLicenseeCode = _rom[AddressConstants::CartridgeOldLicenseeCodeAddress];
}

byte Cartridge::Read(const word address) const
{
    if (address >= _rom.size())
    {
        DEBUGBREAKLOG("Invalid Cartridge ROM read, address: " << std::format("{:x}", address));
        return 0;
    }

    return _rom[address];
}

void Cartridge::Write(word address, byte data)
{
    DEBUGBREAKLOG("Cartridge write not implemented, address: " << std::format("{:x}", address));
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

