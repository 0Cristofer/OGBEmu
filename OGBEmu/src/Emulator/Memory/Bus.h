#pragma once

#include "Core/Definitions.h"

class WRamCgb;
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
    Bus(BootRom* bootRom, Cartridge* cartridge, VRam* vRam, WRam* wRam, WRamCgb* wRamCgb, EchoRam* echoRam, Oam* oam,
        IoRegisters* ioRegisters, HRam* hRam);
    
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
    [[nodiscard]] static byte ReadNotUsed(word address);
    [[nodiscard]] byte ReadIoRegisters(word address) const;
    [[nodiscard]] byte ReadHRam(word address) const;
    [[nodiscard]] byte ReadIe(word address) const;

    void WriteCartridgeBank0(word address, byte data) const;
    static void WriteBootRom(word address, byte data);
    void WriteCartridgeBankN(word address, byte data) const;
    void WriteVRam(word address, byte data) const;
    void WriteExternalRam(word address, byte data) const;
    void WriteWRam(word address, byte data) const;
    void WriteCgbWRam(word address, byte data) const;
    void WriteEchoRam(word address, byte data);
    void WriteOam(word address, byte data) const;
    static void WriteNotUsed(word address, byte data);
    void WriteIoRegisters(word address, byte data);
    void WriteHRam(word address, byte data) const;
    void WriteIe(word address, byte data);

    void DoDma(byte data);
    
    BootRom* _bootRom;
    Cartridge* _cartridge;
    VRam* _vRam;
    WRam* _wRam;
    WRamCgb* _wRamCgb;
    EchoRam* _echoRam;
    Oam* _oam;
    IoRegisters* _ioRegisters;
    HRam* _hRam;
    byte _ie;
};
