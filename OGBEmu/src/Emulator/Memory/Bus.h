#pragma once

#include "Core/Definitions.h"

class HRam;
class Oam;
class EchoRam;
class VRam;
class IoRegisters;
class BootRom;
class Cartridge;
class WRam;

class Bus
{
public:
    Bus(BootRom* bootRom, Cartridge* cartridge, VRam* vRam, WRam* wRam, EchoRam* echoRam, Oam* oam,
        IoRegisters* ioRegisters, HRam* hRam);

    [[nodiscard]] byte& ReadRef(word address);
    [[nodiscard]] byte Read(word address);
    void Write(word address, byte data);

private:
    [[nodiscard]] bool IsBootRomEnabled();
    
    [[nodiscard]] byte& ReadCartridgeBank0(word address);
    [[nodiscard]] byte& ReadBootRom(word address) const;
    [[nodiscard]] byte& ReadCartridgeBankN(word address);
    [[nodiscard]] byte& ReadVRam(word address) const;
    [[nodiscard]] byte& ReadExternalRam(word address);
    [[nodiscard]] byte& ReadWRam(word address) const;
    [[nodiscard]] byte& ReadCgbWRam(word address);
    [[nodiscard]] byte& ReadEchoRam(word address);
    [[nodiscard]] byte& ReadOam(word address) const;
    [[nodiscard]] byte& ReadNotUsed(word address);
    [[nodiscard]] byte& ReadIoRegisters(word address) const;
    [[nodiscard]] byte& ReadHRam(word address) const;
    [[nodiscard]] byte& ReadIe(word address);

    static void WriteCartridgeBank0(word address, byte data);
    static void WriteBootRom(word address, byte data);
    static void WriteCartridgeBankN(word address, byte data);
    void WriteVRam(word address, byte data) const;
    void WriteExternalRam(word address, byte data);
    void WriteWRam(word address, byte data) const;
    void WriteCgbWRam(word address, byte data);
    void WriteEchoRam(word address, byte data);
    void WriteOam(word address, byte data) const;
    void WriteNotUsed(word address, byte data);
    void WriteIoRegisters(word address, byte data) const;
    void WriteHRam(word address, byte data) const;
    void WriteIe(word address, byte data);

    BootRom* _bootRom;
    Cartridge* _cartridge;
    VRam* _vRam;
    WRam* _wRam;
    EchoRam* _echoRam;
    Oam* _oam;
    IoRegisters* _ioRegisters;
    HRam* _hRam;
    byte _ie;
};
