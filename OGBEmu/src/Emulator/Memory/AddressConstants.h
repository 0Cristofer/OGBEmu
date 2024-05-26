#pragma once

#include "Core/Definitions.h"

namespace AddressConstants
{
    // Cartridge rom addresses
    constexpr word CartridgeTypeAddress = 0x1470;

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
    constexpr word StartCgbWRamAddress = 0xD000;
    constexpr word EndCgbWRamAddress = 0xDFFF;
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
