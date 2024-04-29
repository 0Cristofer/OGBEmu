#pragma once

#include "Core/Definitions.h"

union Opcode
{
    byte code;

    struct
    {
        byte column:4;
        byte row:4;
    };
};
