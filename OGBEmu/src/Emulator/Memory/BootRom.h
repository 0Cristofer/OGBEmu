#pragma once

#include <vector>

#include "Core/Definitions.h"

#include "Emulator/GbConstants.h"

class BootRom
{
public:
    explicit BootRom(const std::vector<byte>& romBytes);

    [[nodiscard]] bool IsValid() const { return !_rom.empty() && _rom.size() == GbConstants::BootRomSize; }
    [[nodiscard]] byte Read(word address) const;

private:
    std::vector<byte> _rom;
};
