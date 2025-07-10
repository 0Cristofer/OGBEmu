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
    if (address <= AddressConstants::EndRomBankNAddress)
        return ReadCartridgeBank(address);
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

    ERROR("Trying to read unmapped area, address " << std::format("{:x}", address));
    return 0;
}

void Bus::Write(const word address, const byte data)
{
    if (address <= AddressConstants::EndRomBankNAddress)
        return WriteCartridgeBank(address, data);
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

    ERROR("Trying to write unmapped area, address " << std::format("{:x}", address));
}

bool Bus::IsBootRomEnabled() const
{
    return Read(AddressConstants::BootRomBank) == 0;
}

byte Bus::ReadBootRom(const word address) const
{
    return _bootRom->Read(address);
}

byte Bus::ReadCartridgeBank(const word address) const
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
    // Echo RAM mirrors WRAM - direct access to WRAM data
    const word wramAddress = address - (AddressConstants::StartEchoRamAddress - AddressConstants::StartWRamAddress);
    return ReadWRam(wramAddress);
}

byte Bus::ReadOam(const word address) const
{
    return _oam->Read(address);
}

byte Bus::ReadNotUsed(const word address)
{
    ERROR("Invalid read NotUsed, address " << std::format("{:x}", address));
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

void Bus::WriteBootRom(const word address, const byte data)
{
    ERROR("Invalid write WriteBootRom " << address);
}

void Bus::WriteCartridgeBank(const word address, const byte data) const
{
    if (address <= AddressConstants::EndBootRomAddress)
    {
        if (IsBootRomEnabled())
        {
            return WriteBootRom(address, data);
        }
    }
    
    // Only allow writes to MBC control registers, not ROM data
    // ROM data (0x0000-0x7FFF) should be read-only except for MBC control areas
    if (address >= 0x0000 && address <= 0x7FFF)
    {
        // These are the valid MBC1 control register ranges that can be written to:
        // 0x0000-0x1FFF: RAM Enable
        // 0x2000-0x3FFF: ROM Bank Number  
        // 0x4000-0x5FFF: RAM Bank Number/Upper ROM Bank bits
        // 0x6000-0x7FFF: Banking Mode Select
        
        _cartridge->Write(address, data);
        return;
    }
    
    // External RAM area (0xA000-0xBFFF) - allow writes
    if (address >= AddressConstants::StartExternalRamAddress && address <= AddressConstants::EndExternalRamAddress)
    {
        _cartridge->Write(address, data);
        return;
    }
    
    // All other addresses should not reach the cartridge
    ERROR("Invalid cartridge write attempt to address: " << std::format("{:x}", address) << " data: " << std::format("{:x}", data));
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
}

void Bus::WriteCgbWRam(const word address, const byte data) const
{
    _wRamCgb->Write(address, data);
}

void Bus::WriteEchoRam(const word address, const byte data)
{
    // Echo RAM mirrors WRAM - direct write to WRAM data
    const word wramAddress = address - (AddressConstants::StartEchoRamAddress - AddressConstants::StartWRamAddress);
    WriteWRam(wramAddress, data);
}

void Bus::WriteOam(const word address, const byte data) const
{
    _oam->Write(address, data);
}

void Bus::WriteNotUsed(const word address, const byte data)
{
    ERROR("Invalid write NotUsed address: " << std::format("{:x}", address) << " data: " << std::format("{:x}", data));
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
        Write(AddressConstants::StartOamAddress + i, Read(startAddress + i));
    }
}
