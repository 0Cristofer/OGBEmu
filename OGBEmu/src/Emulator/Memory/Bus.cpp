#include "Bus.h"

#include <format>

#include "EchoRam.h"
#include "WRamCgb.h"
#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/Memory/HRam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/WRam.h"

Bus::Bus(BootRom* bootRom, Cartridge* cartridge, VRam* vRam, WRam* wRam, WRamCgb* wRamCgb, EchoRam* echoRam, Oam* oam,
         IoRegisters* ioRegisters, HRam* hRam) : _bootRom(bootRom),
                                                 _cartridge(cartridge), _vRam(vRam), _wRam(wRam), _wRamCgb(wRamCgb), _echoRam(echoRam),
                                                 _oam(oam),
                                                 _ioRegisters(ioRegisters),
                                                 _hRam(hRam), _ie(0)
{
}

byte Bus::Read(const word address) const
{
    if (address <= AddressConstants::EndRomBank0Address)
        return ReadCartridgeBank0(address);
    if (address >= AddressConstants::StartRomBankNAddress && address <= AddressConstants::EndRomBankNAddress)
        return ReadCartridgeBankN(address);
    if (address >= AddressConstants::StartVRamAddress && address <= AddressConstants::EndVRamAddress)
        return ReadVRam(address);
    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
        return ReadExternalRam(address);
    if (address >= AddressConstants::StartWRamAddress && address <= AddressConstants::EndWRamAddress)
        return ReadWRam(address);
    if (address >= AddressConstants::StartWRamCgbAddress && address <= AddressConstants::EndWRamCgbAddress)
        return ReadCgbWRam(address);
    if (address >= AddressConstants::StartEchoRamAddress && address <= AddressConstants::EndEchoRamAddress)
        return ReadEchoRam(address);
    if (address >= AddressConstants::StartOamAddress && address <= AddressConstants::EndOamAddress)
        return ReadOam(address);
    if (address >= AddressConstants::StartNotUsedAddress && address <= AddressConstants::EndNotUsedAddress)
        return ReadNotUsed(address);
    if (address >= AddressConstants::StartIoRegistersAddress && address <= AddressConstants::EndIoRegistersAddress)
        return ReadIoRegisters(address);
    if (address >= AddressConstants::StartHRamAddress && address <= AddressConstants::EndHRamAddress)
        return ReadHRam(address);
    if (address >= AddressConstants::StartIeAddress)
        return ReadIe(address);

    DEBUGBREAKLOG("Trying to read unmapped area, address " << std::format("{:x}", address));
    return 0;
}

void Bus::Write(const word address, const byte data)
{
    if (address <= AddressConstants::EndRomBank0Address)
        return WriteCartridgeBank0(address, data);
    if (address >= AddressConstants::StartRomBankNAddress && address <= AddressConstants::EndRomBankNAddress)
        return WriteCartridgeBankN(address, data);
    if (address >= AddressConstants::StartVRamAddress && address <= AddressConstants::EndVRamAddress)
        return WriteVRam(address, data);
    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
        return WriteExternalRam(address, data);
    if (address >= AddressConstants::StartWRamAddress && address <= AddressConstants::EndWRamAddress)
        return WriteWRam(address, data);
    if (address >= AddressConstants::StartWRamCgbAddress && address <= AddressConstants::EndWRamCgbAddress)
        return WriteCgbWRam(address, data);
    if (address >= AddressConstants::StartEchoRamAddress && address <= AddressConstants::EndEchoRamAddress)
        return WriteEchoRam(address, data);
    if (address >= AddressConstants::StartOamAddress && address <= AddressConstants::EndOamAddress)
        return WriteOam(address, data);
    if (address >= AddressConstants::StartNotUsedAddress && address <= AddressConstants::EndNotUsedAddress)
        return WriteNotUsed(address, data);
    if (address >= AddressConstants::StartIoRegistersAddress && address <= AddressConstants::EndIoRegistersAddress)
        return WriteIoRegisters(address, data);
    if (address >= AddressConstants::StartHRamAddress && address <= AddressConstants::EndHRamAddress)
        return WriteHRam(address, data);
    if (address >= AddressConstants::StartIeAddress)
        return WriteIe(address, data);

    DEBUGBREAKLOG("Trying to write unmapped area, address " << std::format("{:x}", address));
}

bool Bus::IsBootRomEnabled() const
{
    return Read(AddressConstants::BootRomBank) == 0;
}

byte Bus::ReadCartridgeBank0(const word address) const
{
    if (address <= AddressConstants::EndBootRomAddress)
    {
        if (IsBootRomEnabled())
        {
            return ReadBootRom(address);
        }
    }
    
    return _cartridge->Read(address);
}

byte Bus::ReadBootRom(const word address) const
{
    return _bootRom->Read(address);
}

byte Bus::ReadCartridgeBankN(const word address) const
{
    return _cartridge->Read(address);
}

byte Bus::ReadVRam(const word address) const
{
    return _vRam->Read(address);
}

byte Bus::ReadExternalRam(const word address) const
{
    return _cartridge->Read(address);
}

byte Bus::ReadWRam(const word address) const
{
    return _wRam->Read(address);
}

byte Bus::ReadCgbWRam(const word address) const
{
    return _wRamCgb->Read(address);
}

byte Bus::ReadEchoRam(const word address) const
{
    return _echoRam->Read(address);
}

byte Bus::ReadOam(const word address) const
{
    return _oam->Read(address);
}

byte Bus::ReadNotUsed(const word address)
{
    DEBUGBREAKLOG("Invalid read NotUsed, address " << std::format("{:x}", address));
    return 0;
}

byte Bus::ReadIoRegisters(const word address) const
{
    return _ioRegisters->Read(address);
}

byte Bus::ReadHRam(const word address) const
{
    return _hRam->Read(address);
}

byte Bus::ReadIe(const word address) const
{
    return _ie;
}

void Bus::WriteCartridgeBank0(const word address, const byte data) const
{
    if (address <= AddressConstants::EndBootRomAddress)
    {
        if (IsBootRomEnabled())
        {
            return WriteBootRom(address, data);
        }
    }
    
    _cartridge->Write(address, data);
}

void Bus::WriteBootRom(const word address, const byte data)
{
    DEBUGBREAKLOG("Invalid write WriteBootRom " << address);
}

void Bus::WriteCartridgeBankN(const word address, const byte data) const
{
    _cartridge->Write(address, data);
}

void Bus::WriteVRam(const word address, const byte data) const
{
    _vRam->Write(address, data);
}

void Bus::WriteExternalRam(const word address, const byte data) const
{
    _cartridge->Write(address, data);
}

void Bus::WriteWRam(const word address, const byte data) const
{
    _wRam->Write(address, data);
    
    const word echoRamAddress = address + (AddressConstants::StartEchoRamAddress - AddressConstants::StartWRamAddress);
    if (echoRamAddress <= AddressConstants::EndEchoRamAddress)
        _echoRam->Write(echoRamAddress, data);
}

void Bus::WriteCgbWRam(const word address, const byte data) const
{
    _wRamCgb->Write(address, data);

    const word echoRamAddress = address + (AddressConstants::StartEchoRamAddress - AddressConstants::StartWRamAddress);
    if (echoRamAddress <= AddressConstants::EndEchoRamAddress)
        _echoRam->Write(echoRamAddress, data);
}

void Bus::WriteEchoRam(const word address, const byte data)
{
    DEBUGBREAKLOG("Invalid write EchoRam " << std::format("{:x}", address));
    Write(address - (AddressConstants::StartEchoRamAddress - AddressConstants::StartWRamAddress), data);
}

void Bus::WriteOam(const word address, const byte data) const
{
    _oam->Write(address, data);
}

void Bus::WriteNotUsed(const word address, const byte data)
{
    DEBUGBREAKLOG("Invalid write NotUsed " << std::format("{:x}", address));
}

void Bus::WriteIoRegisters(const word address, const byte data)
{
    _ioRegisters->Write(address, data);

    if (address == AddressConstants::DmaStart)
        DoDma(data);
}

void Bus::WriteHRam(const word address, const byte data) const
{
    _hRam->Write(address, data);
}

void Bus::WriteIe(const word address, const byte data)
{
    _ie = data;
}

void Bus::DoDma(const byte data)
{
    const word startAddress = static_cast<word>(data << 8);
    constexpr word oamRange = AddressConstants::EndOamAddress - AddressConstants::StartOamAddress + 1;

    for (word i = 0; i < oamRange; i++)
    {
        Write(AddressConstants::StartOamAddress + 0, Read(startAddress + i));
    }
}
