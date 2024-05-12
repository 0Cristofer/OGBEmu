#include "Bus.h"

#include "HRam.h"
#include "Oam.h"
#include "VRam.h"
#include "WRam.h"
#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/IoRegisters.h"

Bus::Bus(BootRom* bootRom, Cartridge* cartridge, VRam* vRam, WRam* wRam, EchoRam* echoRam, Oam* oam,
         IoRegisters* ioRegisters, HRam* hRam) : _bootRom(bootRom),
                                                 _cartridge(cartridge), _vRam(vRam), _wRam(wRam), _echoRam(echoRam),
                                                 _oam(oam),
                                                 _ioRegisters(ioRegisters),
                                                 _hRam(hRam), _ie(0)
{
}

byte& Bus::ReadRef(const word address) const
{
    if (address <= AddressConstants::EndRomBank0Address)
    {
        return ReadCartridgeBank0(address);
    }

    if (address >= AddressConstants::StartRomBankNAddress && address <= AddressConstants::EndRomBankNAddress)
    {
        return ReadCartridgeBankN(address);
    }

    if (address >= AddressConstants::StartVRamAddress && address <= AddressConstants::EndVRamAddress)
    {
        return ReadVRam(address);
    }

    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
    {
        return ReadExternalRam(address);
    }

    if (address >= AddressConstants::StartWRamAddress && address <= AddressConstants::EndWRamAddress)
    {
        return ReadWRam(address);
    }

    if (address >= AddressConstants::StartCgbWRamAddress && address <= AddressConstants::EndCgbWRamAddress)
    {
        return ReadCgbWRam(address);
    }

    if (address >= AddressConstants::StartEchoRamAddress && address <= AddressConstants::EndEchoRamAddress)
    {
        return ReadEchoRam(address);
    }

    if (address >= AddressConstants::StartOamAddress && address <= AddressConstants::EndOamAddress)
    {
        return ReadOam(address);
    }

    if (address >= AddressConstants::StartNotUsedAddress && address <= AddressConstants::EndNotUsedAddress)
    {
        return ReadNotUsed(address);
    }

    if (address >= AddressConstants::StartIoRegistersAddress && address <= AddressConstants::EndIoRegistersAddress)
    {
        return ReadIoRegisters(address);
    }

    if (address >= AddressConstants::StartHRamAddress && address <= AddressConstants::EndHRamAddress)
    {
        return ReadHRam(address);
    }

    if (address >= AddressConstants::StartIeAddress)
    {
        return ReadIe(address);
    }

    LOG("Trying to read unmapped area, address " << address);
    return ReadRef(0);
}

byte Bus::Read(const word address) const
{
    return ReadRef(address);
}

void Bus::Write(const word address, const byte data)
{
    if (address <= AddressConstants::EndRomBank0Address)
    {
        WriteCartridgeBank0(address, data);
        return;
    }

    if (address >= AddressConstants::StartRomBankNAddress && address <= AddressConstants::EndRomBankNAddress)
    {
        WriteCartridgeBankN(address, data);
        return;
    }

    if (address >= AddressConstants::StartVRamAddress && address <= AddressConstants::EndVRamAddress)
    {
        WriteVRam(address, data);
        return;
    }

    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
    {
        WriteExternalRam(address, data);
        return;
    }

    if (address >= AddressConstants::StartWRamAddress && address <= AddressConstants::EndWRamAddress)
    {
        WriteWRam(address, data);
        return;
    }

    if (address >= AddressConstants::StartCgbWRamAddress && address <= AddressConstants::EndCgbWRamAddress)
    {
        WriteCgbWRam(address, data);
        return;
    }

    if (address >= AddressConstants::StartEchoRamAddress && address <= AddressConstants::EndEchoRamAddress)
    {
        WriteEchoRam(address, data);
        return;
    }

    if (address >= AddressConstants::StartOamAddress && address <= AddressConstants::EndOamAddress)
    {
        WriteOam(address, data);
        return;
    }

    if (address >= AddressConstants::StartNotUsedAddress && address <= AddressConstants::EndNotUsedAddress)
    {
        WriteNotUsed(address, data);
        return;
    }

    if (address >= AddressConstants::StartIoRegistersAddress && address <= AddressConstants::EndIoRegistersAddress)
    {
        WriteIoRegisters(address, data);
        return;
    }

    if (address >= AddressConstants::StartHRamAddress && address <= AddressConstants::EndHRamAddress)
    {
        WriteHRam(address, data);
        return;
    }

    if (address >= AddressConstants::StartIeAddress)
    {
        WriteIe(address, data);
        return;
    }

    LOG("Trying to write unmapped area, address " << address);
}

bool Bus::IsBootRomEnabled() const
{
    return Read(AddressConstants::BootRomBank) == 0;
}

byte& Bus::ReadCartridgeBank0(const word address) const
{
    if (address <= AddressConstants::EndBootRomAddress)
    {
        if (IsBootRomEnabled())
        {
            return ReadBootRom(address);
        }
    }
    
    LOG("ReadCartridgeBank0 not fully implemented, address " << address);
    return ReadRef(0);
}

byte& Bus::ReadBootRom(const word address) const
{
    return _bootRom->ReadRef(address);
}

byte& Bus::ReadCartridgeBankN(const word address) const
{
    LOG("ReadCartridgeBankN not implemented, address " << address);
    return ReadRef(0);
}

byte& Bus::ReadVRam(const word address) const
{
    return _vRam->ReadRef(address);
}

byte& Bus::ReadExternalRam(const word address) const
{
    LOG("ReadExternalRam not implemented, address " << address);
    return ReadRef(0);
}

byte& Bus::ReadWRam(const word address) const
{
    return _wRam->ReadRef(address);
}

byte& Bus::ReadCgbWRam(const word address) const
{
    LOG("ReadCgbWRam not implemented, address " << address);
    return ReadRef(0);
}

byte& Bus::ReadEchoRam(const word address) const
{
    LOG("ReadEchoRam not implemented, address " << address);
    return ReadRef(0);
}

byte& Bus::ReadOam(const word address) const
{
    return _oam->ReadRef(address);
}

byte& Bus::ReadNotUsed(const word address) const
{
    LOG("ReadNotUsed not implemented, address " << address);
    return ReadRef(0);
}

byte& Bus::ReadIoRegisters(const word address) const
{
    return _ioRegisters->ReadRef(address);
}

byte& Bus::ReadHRam(const word address) const
{
    return _hRam->ReadRef(address);
}

byte& Bus::ReadIe(const word address) const
{
    LOG("ReadIe not implemented, address " << address);
    return ReadRef(0);
}

void Bus::WriteCartridgeBank0(const word address, const byte data)
{
    LOG("Invalid write WriteCartridgeBank0" << address);
}

void Bus::WriteBootRom(const word address, const byte data)
{
    LOG("Invalid write WriteBootRom" << address);
}

void Bus::WriteCartridgeBankN(const word address, const byte data)
{
    LOG("Invalid write WriteCartridgeBankN" << address);
}

void Bus::WriteVRam(const word address, const byte data) const
{
    _vRam->Write(address, data);
}

void Bus::WriteExternalRam(const word address, const byte data)
{
    LOG("Invalid write WriteExternalRam" << address);
}

void Bus::WriteWRam(const word address, const byte data) const
{
    _wRam->Write(address, data);
}

void Bus::WriteCgbWRam(const word address, const byte data)
{
    LOG("Invalid write WriteCgbWRam" << address);
}

void Bus::WriteEchoRam(const word address, const byte data)
{
    LOG("Invalid write WriteEchoRam" << address);
}

void Bus::WriteOam(const word address, const byte data) const
{
    _oam->Write(address, data);
}

void Bus::WriteNotUsed(const word address, const byte data)
{
    LOG("Invalid write WriteNotUsed" << address);
}

void Bus::WriteIoRegisters(const word address, const byte data) const
{
    _ioRegisters->Write(address, data);
}

void Bus::WriteHRam(const word address, const byte data) const
{
    _hRam->Write(address, data);
}

void Bus::WriteIe(const word address, const byte data)
{
    LOG("Invalid write WriteIe" << address);
}
