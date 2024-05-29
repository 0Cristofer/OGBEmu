#pragma once

#include "Core/Definitions.h"

namespace AddressConstants
{
    // Cartridge rom addresses
    constexpr word CartridgeTitleStartAddress = 0x134;
    constexpr word CartridgeTitleNewEndAddress = 0x13F;
    constexpr word CartridgeTitleOldEndAddress = 0x143;
    constexpr word CartridgeManufacturerCodeStartAddress = 0x13F;
    constexpr word CartridgeManufacturerCodeEndAddress = 0x142;
    constexpr word CartridgeCgbFlagAddress = 0x143;
    constexpr word CartridgeNewLicenseeCodeStartAddress = 0x144;
    constexpr word CartridgeNewLicenseeCodeEndAddress = 0x145;
    constexpr word CartridgeSgbFlagAddress = 0x146;
    constexpr word CartridgeTypeAddress = 0x147;
    constexpr word CartridgeRomSizeAddress = 0x148;
    constexpr word CartridgeRamSizeAddress = 0x149;
    constexpr word CartridgeDestinationCodeAddress = 0x14A;
    constexpr word CartridgeOldLicenseeCodeAddress = 0x14B;
    constexpr word CartridgeMaskRomVersionNumberAddress = 0x14C;
    constexpr word CartridgeHeaderChecksumAddress = 0x14D;
    constexpr word CartridgeGlobalChecksumAddressStart = 0x14E;
    constexpr word CartridgeGlobalChecksumAddressEnd = 0x14F;

    // Bus addresses
    constexpr word StartBootRomAddress = 0x0000;
    constexpr word EndBootRomAddress = 0x00FF;
    constexpr word StartRomBank0Address = 0x0000;
    constexpr word EndRomBank0Address = 0x3FFF;
    constexpr word StartRomBankNAddress = 0x4000;
    constexpr word EndRomBankNAddress = 0x7FFF;
    constexpr word StartVRamAddress = 0x8000;
    constexpr word EndVRamAddress = 0x9FFF;
    constexpr word StartExternalRamAddress = 0xA000;
    constexpr word EndExternalRamAddress = 0xBFFF;
    constexpr word StartWRamAddress = 0xC000;
    constexpr word EndWRamAddress = 0xCFFF;
    constexpr word StartWRamCgbAddress = 0xD000;
    constexpr word EndWRamCgbAddress = 0xDFFF;
    constexpr word StartEchoRamAddress = 0xE000;
    constexpr word EndEchoRamAddress = 0xFDFF;
    constexpr word StartOamAddress = 0xFE00;
    constexpr word EndOamAddress = 0xFE9F;
    constexpr word StartNotUsedAddress = 0xFEA0;
    constexpr word EndNotUsedAddress = 0xFEFF;
    constexpr word StartIoRegistersAddress = 0xFF00;
    constexpr word EndIoRegistersAddress = 0xFF7F;
    constexpr word StartHRamAddress = 0xFF80;
    constexpr word EndHRamAddress = 0xFFFE;
    constexpr word StartIeAddress = 0xFFFF;
    constexpr word EndIeAddress = 0xFFFF;

    // IO addresses
    constexpr word InterruptFlag = 0xFF0F;
    constexpr word DmaStart = 0xFF46;
    constexpr word BootRomBank = 0xFF50;

    // Interrupt handler addresses (ISR)
    constexpr word VBlankHandlerAddress = 0x40;
    constexpr word LcdHandlerAddress = 0x48;
    constexpr word TimerHandlerAddress = 0x50;
    constexpr word SerialHandlerAddress = 0x58;
    constexpr word JoypadHandlerAddress = 0x60;
};
