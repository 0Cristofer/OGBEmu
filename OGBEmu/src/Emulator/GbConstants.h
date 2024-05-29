#pragma once

#include "Core/Definitions.h"

namespace GbConstants
{
    // Sizes
    constexpr word BootRomSize = 256;
    constexpr word MinCartridgeRomSize = 32 * 1024;

    // Flags values
    constexpr byte CgbFlag = 0xC0;
    constexpr byte NewLicenseeCode = 0x33;
};
