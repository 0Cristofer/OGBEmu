#pragma once

#include <vector>

#include "Core/Definitions.h"

class EchoRam
{
public:
    EchoRam();
    
    [[nodiscard]] byte Read(word busAddress);
    void Write(word busAddress, byte data);

private:
    static word TranslateAddress(word busAddress);

    std::vector<byte> _bytes;
};