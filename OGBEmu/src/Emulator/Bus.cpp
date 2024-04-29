#include "Bus.h"

#include "AddressConstants.h"
#include "BootRom.h"
#include "GbConstants.h"
#include "IoRegisters.h"
#include "Core/Logger.h"

Bus::Bus(BootRom* bootRom, Cartridge* cartridge, Ram* ram, IoRegisters* ioRegisters) : _bootRom(bootRom), _cartridge(cartridge), _ram(ram), _ioRegisters(ioRegisters)
{
}

byte Bus::Read(const word address) const
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
    return 0;
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

byte Bus::ReadCartridgeBank0(const word address) const
{
    if (address <= AddressConstants::EndBootRomAddress)
    {
        if (IsBootRomEnabled())
        {
            return ReadBootRom(address);
        }
    }
    
    LOG("ReadCartridgeBank0 not fully implemented, address " << address);
    return 0;
}

byte Bus::ReadBootRom(const word address) const
{
    return _bootRom->Read(address);
}

byte Bus::ReadCartridgeBankN(const word address) const
{
    LOG("ReadCartridgeBankN not implemented, address " << address);
    return 0;
}

byte Bus::ReadVRam(const word address) const
{
    LOG("ReadVRam not implemented, address " << address);
    return 0;
}

byte Bus::ReadExternalRam(const word address) const
{
    LOG("ReadExternalRam not implemented, address " << address);
    return 0;
}

byte Bus::ReadWRam(const word address) const
{
    LOG("ReadWRam not implemented, address " << address);
    return 0;
}

byte Bus::ReadCgbWRam(const word address) const
{
    LOG("ReadCgbWRam not implemented, address " << address);
    return 0;
}

byte Bus::ReadEchoRam(const word address) const
{
    LOG("ReadEchoRam not implemented, address " << address);
    return 0;
}

byte Bus::ReadOam(const word address) const
{
    LOG("ReadOam not implemented, address " << address);
    return 0;
}

byte Bus::ReadNotUsed(const word address) const
{
    LOG("ReadNotUsed not implemented, address " << address);
    return 0;
}

byte Bus::ReadIoRegisters(const word address) const
{
    return _ioRegisters->Read(address);
}

byte Bus::ReadHRam(const word address) const
{
    LOG("ReadHRam not implemented, address " << address);
    return 0;
}

byte Bus::ReadIe(const word address) const
{
    LOG("ReadIe not implemented, address " << address);
    return 0;
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

void Bus::WriteVRam(const word address, const byte data)
{
    LOG("Invalid write WriteVRam" << address);
}

void Bus::WriteExternalRam(const word address, const byte data)
{
    LOG("Invalid write WriteExternalRam" << address);
}

void Bus::WriteWRam(const word address, const byte data)
{
    LOG("Invalid write WriteWRam" << address);
}

void Bus::WriteCgbWRam(const word address, const byte data)
{
    LOG("Invalid write WriteCgbWRam" << address);
}

void Bus::WriteEchoRam(const word address, const byte data)
{
    LOG("Invalid write WriteEchoRam" << address);
}

void Bus::WriteOam(const word address, const byte data)
{
    LOG("Invalid write WriteOam" << address);
}

void Bus::WriteNotUsed(const word address, const byte data)
{
    LOG("Invalid write WriteNotUsed" << address);
}

void Bus::WriteIoRegisters(const word address, const byte data)
{
    _ioRegisters->Write(address, data);
}

void Bus::WriteHRam(const word address, const byte data)
{
    LOG("Invalid write WriteHRam" << address);
}

void Bus::WriteIe(const word address, const byte data)
{
    LOG("Invalid write WriteIe" << address);
}
