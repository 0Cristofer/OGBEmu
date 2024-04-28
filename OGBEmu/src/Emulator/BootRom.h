#pragma once

#include <vector>

#include "GbConstants.h"

class BootRom
{
public:
    explicit BootRom(std::vector<unsigned char> romBytes);

    [[nodiscard]] bool IsValid() const { return !_rom.empty() && _rom.size() != GbConstants::MinCartridgeRomSize; }

private:
    std::vector<unsigned char> _rom;   
};
