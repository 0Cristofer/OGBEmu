#pragma once

#include <string>
#include <vector>

#include "Definitions.h"

namespace Utils
{
    std::vector<byte> ReadBinaryFile(const std::string& filePath);
};
