#pragma once

#include "Core/Definitions.h"

class BootRom;
class Cartridge;
class Ram;

class Bus
{
public:
    Bus(BootRom& bootRom, Cartridge& cartridge, Ram& ram);

    void EnableBootRom() { _bootRomEnabled = true; }
    void DisableBootRom() { _bootRomEnabled = false; }
    
    [[nodiscard]] byte Read(const word address) const;
    void Write(const word address, const byte data);

private:
    [[nodiscard]] byte ReadCartridgeBank0(const word address) const;
    [[nodiscard]] byte ReadBootRom(const word address) const;
    [[nodiscard]] byte ReadCartridgeBankN(const word address) const;
    [[nodiscard]] byte ReadVRam(const word address) const;
    [[nodiscard]] byte ReadExternalRam(const word address) const;
    [[nodiscard]] byte ReadWRam(const word address) const;
    [[nodiscard]] byte ReadCgbWRam(const word address) const;
    [[nodiscard]] byte ReadEchoRam(const word address) const;
    [[nodiscard]] byte ReadOam(const word address) const;
    [[nodiscard]] byte ReadNotUsed(const word address) const;
    [[nodiscard]] byte ReadIoRegisters(const word address) const;
    [[nodiscard]] byte ReadHRam(const word address) const;
    [[nodiscard]] byte ReadIe(const word address) const;

    void WriteCartridgeBank0(const word address, const byte data);
    void WriteBootRom(const word address, const byte data);
    void WriteCartridgeBankN(const word address, const byte data);
    void WriteVRam(const word address, const byte data);
    void WriteExternalRam(const word address, const byte data);
    void WriteWRam(const word address, const byte data);
    void WriteCgbWRam(const word address, const byte data);
    void WriteEchoRam(const word address, const byte data);
    void WriteOam(const word address, const byte data);
    void WriteNotUsed(const word address, const byte data);
    void WriteIoRegisters(const word address, const byte data);
    void WriteHRam(const word address, const byte data);
    void WriteIe(const word address, const byte data);

    BootRom& _bootRom;
    Cartridge& _cartridge;
    Ram& _ram;

    bool _bootRomEnabled;
};
