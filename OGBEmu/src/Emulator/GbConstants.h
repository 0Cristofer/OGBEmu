#pragma once

#include "Core/Definitions.h"

namespace GbConstants
{
    // Sizes
    constexpr word BootRomSize = 256;
    constexpr word MinCartridgeRomSize = 32 * 1024;
    constexpr word RomBankSize = 32 * 1024;
    constexpr word RamBankSize = 8 * 1024;

    // Flags values
    constexpr byte CgbFlag = 0xC0;
    constexpr byte NewLicenseeCode = 0x33;
    constexpr byte RamSizeFlagNoRam = 0x0;
    constexpr byte RamSizeFlag1Bank = 0x2;
    constexpr byte RamSizeFlag4Bank = 0x3;
    constexpr byte RamSizeFlag16Bank = 0x4;
    constexpr byte RamSizeFlag8Bank = 0x5;
};
