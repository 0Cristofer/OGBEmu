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
        byte column2:2;
        byte row6:6;
    };
};
