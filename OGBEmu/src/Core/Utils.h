#pragma once

#include <string>
#include <vector>

#include "Definitions.h"

namespace Utils
{
    std::vector<byte> ReadBinaryFile(const std::string& filePath);
    inline bool IsPowerOfTwo(const unsigned int value) { return value != 0 && (value & value - 1) == 0; }
};
