#include "Bus.h"

#include "BootRom.h"
#include "GbConstants.h"
#include "Core/Logger.h"

Bus::Bus(BootRom& bootRom, Cartridge& cartridge, Ram& ram) : _bootRom(bootRom), _cartridge(cartridge), _ram(ram), _bootRomEnabled(false)
{
}

byte Bus::Read(const word address) const
{
    if (address <= GbConstants::EndRomBank0Address)
    {
        return ReadCartridgeBank0(address);
    }

    if (address >= GbConstants::StartRomBankNAddress && address <= GbConstants::EndRomBankNAddress)
    {
        return ReadCartridgeBankN(address);
    }

    if (address >= GbConstants::StartVRamAddress && address <= GbConstants::EndVRamAddress)
    {
        return ReadVRam(address);
    }

    if (address >= GbConstants::StartExternalRamAddress && address <= GbConstants::EndExternalRamAddress)
    {
        return ReadExternalRam(address);
    }

    if (address >= GbConstants::StartWRamAddress && address <= GbConstants::EndWRamAddress)
    {
        return ReadWRam(address);
    }

    if (address >= GbConstants::StartCgbWRamAddress && address <= GbConstants::EndCgbWRamAddress)
    {
        return ReadCgbWRam(address);
    }

    if (address >= GbConstants::StartEchoRamAddress && address <= GbConstants::EndEchoRamAddress)
    {
        return ReadEchoRam(address);
    }

    if (address >= GbConstants::StartOamAddress && address <= GbConstants::EndOamAddress)
    {
        return ReadOam(address);
    }

    if (address >= GbConstants::StartNotUsedAddress && address <= GbConstants::EndNotUsedAddress)
    {
        return ReadNotUsed(address);
    }

    if (address >= GbConstants::StartIoRegistersAddress && address <= GbConstants::EndIoRegistersAddress)
    {
        return ReadIoRegisters(address);
    }

    if (address >= GbConstants::StartHRamAddress && address <= GbConstants::EndHRamAddress)
    {
        return ReadHRam(address);
    }

    if (address >= GbConstants::StartIeAddress)
    {
        return ReadIe(address);
    }

    LOG("Trying to read unmapped area, address " << address);
    return 0;
}

void Bus::Write(const word address, const byte data)
{
    if (address <= GbConstants::EndRomBank0Address)
    {
        WriteCartridgeBank0(address, data);
        return;
    }

    if (address >= GbConstants::StartRomBankNAddress && address <= GbConstants::EndRomBankNAddress)
    {
        WriteCartridgeBankN(address, data);
        return;
    }

    if (address >= GbConstants::StartVRamAddress && address <= GbConstants::EndVRamAddress)
    {
        WriteVRam(address, data);
        return;
    }

    if (address >= GbConstants::StartExternalRamAddress && address <= GbConstants::EndExternalRamAddress)
    {
        WriteExternalRam(address, data);
        return;
    }

    if (address >= GbConstants::StartWRamAddress && address <= GbConstants::EndWRamAddress)
    {
        WriteWRam(address, data);
        return;
    }

    if (address >= GbConstants::StartCgbWRamAddress && address <= GbConstants::EndCgbWRamAddress)
    {
        WriteCgbWRam(address, data);
        return;
    }

    if (address >= GbConstants::StartEchoRamAddress && address <= GbConstants::EndEchoRamAddress)
    {
        WriteEchoRam(address, data);
        return;
    }

    if (address >= GbConstants::StartOamAddress && address <= GbConstants::EndOamAddress)
    {
        WriteOam(address, data);
        return;
    }

    if (address >= GbConstants::StartNotUsedAddress && address <= GbConstants::EndNotUsedAddress)
    {
        WriteNotUsed(address, data);
        return;
    }

    if (address >= GbConstants::StartIoRegistersAddress && address <= GbConstants::EndIoRegistersAddress)
    {
        WriteIoRegisters(address, data);
        return;
    }

    if (address >= GbConstants::StartHRamAddress && address <= GbConstants::EndHRamAddress)
    {
        WriteHRam(address, data);
        return;
    }

    if (address >= GbConstants::StartIeAddress)
    {
        WriteIe(address, data);
        return;
    }

    LOG("Trying to write unmapped area, address " << address);
}

byte Bus::ReadCartridgeBank0(const word address) const
{
    if (address <= GbConstants::EndBootRomAddress)
    {
        if (_bootRomEnabled)
        {
            return ReadBootRom(address);
        }
    }
    
    LOG("ReadCartridgeBank0 not fully implemented, address " << address);
    return 0;
}

byte Bus::ReadBootRom(const word address) const
{
    return _bootRom.Read(address);
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
    LOG("ReadIoRegisters not implemented, address " << address);
    return 0;
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
    LOG("Invalid write WriteIoRegisters" << address);
}

void Bus::WriteHRam(const word address, const byte data)
{
    LOG("Invalid write WriteHRam" << address);
}

void Bus::WriteIe(const word address, const byte data)
{
    LOG("Invalid write WriteIe" << address);
}
