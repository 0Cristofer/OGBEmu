#pragma once

#include <vector>

#include "GbConstants.h"
#include "Core/Definitions.h"

class BootRom
{
public:
    explicit BootRom(std::vector<byte> romBytes);

    [[nodiscard]] bool IsValid() const { return !_rom.empty() && _rom.size() == GbConstants::BootRomSize; }
    [[nodiscard]] byte Read(word address) const;

private:
    std::vector<byte> _rom;
};
