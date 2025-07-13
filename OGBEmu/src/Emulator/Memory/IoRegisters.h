#pragma once

#include <vector>

#include "Core/Definitions.h"

// Forward declaration
class Apu;

class IoRegisters
{
public:
    IoRegisters();
    
    [[nodiscard]] byte Read(word busAddress) const;
    void Write(word busAddress, byte data);
    
    // Set APU for audio register callbacks
    void SetApu(Apu* apu) { _apu = apu; }

private:
    static word TranslateAddress(word busAddress);

    std::vector<byte> _registers;
    Apu* _apu;
};
