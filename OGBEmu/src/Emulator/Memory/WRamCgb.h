#pragma once

#include <vector>

#include "Core/Definitions.h"

class WRamCgb
{
public:
    WRamCgb();
    
    [[nodiscard]] byte Read(word busAddress) const;
    void Write(word busAddress, byte data);

private:
    static word TranslateAddress(word busAddress);

    std::vector<byte> _bytes;
};
