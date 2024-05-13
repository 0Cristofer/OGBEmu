#include "Utils.h"

#include <fstream>

#include "Logger.h"

std::vector<byte> Utils::ReadBinaryFile(const std::string& filePath)
{
    std::ifstream file;
    
    file.open(filePath, std::ifstream::binary);

    if (!file.good())
    {
        LOG("Error reading file " << filePath);

        file.close();

        return {};
    }

    return {std::istreambuf_iterator(file), {}};
}
