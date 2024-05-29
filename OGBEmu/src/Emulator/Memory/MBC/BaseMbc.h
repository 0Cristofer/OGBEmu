#pragma once

#include "Core/Definitions.h"

class BaseMbc
{
public:
    virtual ~BaseMbc() = default;
    
    virtual byte Read(word address) = 0;
    virtual void Write(word address, byte data) = 0;
};
