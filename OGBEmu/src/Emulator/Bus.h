#pragma once

#include "Core/Definitions.h"

class IoRegisters;
class BootRom;
class Cartridge;
class Ram;

class Bus
{
public:
    Bus(BootRom* bootRom, Cartridge* cartridge, Ram* ram, IoRegisters* ioRegisters);
    
    [[nodiscard]] byte Read(word address) const;
    void Write(word address, byte data);

private:
    [[nodiscard]] bool IsBootRomEnabled() const;
    
    [[nodiscard]] byte ReadCartridgeBank0(word address) const;
    [[nodiscard]] byte ReadBootRom(word address) const;
    [[nodiscard]] byte ReadCartridgeBankN(word address) const;
    [[nodiscard]] byte ReadVRam(word address) const;
    [[nodiscard]] byte ReadExternalRam(word address) const;
    [[nodiscard]] byte ReadWRam(word address) const;
    [[nodiscard]] byte ReadCgbWRam(word address) const;
    [[nodiscard]] byte ReadEchoRam(word address) const;
    [[nodiscard]] byte ReadOam(word address) const;
    [[nodiscard]] byte ReadNotUsed(word address) const;
    [[nodiscard]] byte ReadIoRegisters(word address) const;
    [[nodiscard]] byte ReadHRam(word address) const;
    [[nodiscard]] byte ReadIe(word address) const;

    void WriteCartridgeBank0(word address, byte data);
    void WriteBootRom(word address, byte data);
    void WriteCartridgeBankN(word address, byte data);
    void WriteVRam(word address, byte data);
    void WriteExternalRam(word address, byte data);
    void WriteWRam(word address, byte data);
    void WriteCgbWRam(word address, byte data);
    void WriteEchoRam(word address, byte data);
    void WriteOam(word address, byte data);
    void WriteNotUsed(word address, byte data);
    void WriteIoRegisters(word address, byte data);
    void WriteHRam(word address, byte data);
    void WriteIe(word address, byte data);

    BootRom* _bootRom;
    Cartridge* _cartridge;
    Ram* _ram;
    IoRegisters* _ioRegisters;
};
