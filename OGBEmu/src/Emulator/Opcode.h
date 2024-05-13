#pragma once

#include "Core/Definitions.h"

union Opcode
{
    byte code;

    struct
    {
        byte low:4;
        byte high:4;
    };

    struct
    {
        byte column:4;
        byte row:4;
    };

    struct
    {
        byte column3:3;
        byte row5:5;
    };
};
