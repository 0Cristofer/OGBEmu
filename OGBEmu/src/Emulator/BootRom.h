#pragma once

#include <vector>

#include "GbConstants.h"
#include "Core/Definitions.h"

class BootRom
{
public:
    explicit BootRom(std::vector<byte> romBytes);

    [[nodiscard]] bool IsValid() const { return !_rom.empty() && _rom.size() == GbConstants::BootRomSize; }
    [[nodiscard]] byte& ReadRef(word address);
    [[nodiscard]] byte Read(word address);

private:
    std::vector<byte> _rom;
};
