#include "Cartridge.h"

#include <fstream>

#include "Logger.h"

Cartridge::Cartridge(const std::string& romPath)
{
    _bytes = ReadRom(romPath);
}

std::vector<unsigned char> Cartridge::ReadRom(const std::string& romPath)
{
    std::ifstream romFile;
    
    romFile.open(romPath, std::ifstream::binary);

    if (!romFile.good())
    {
        LOG("Error reading file " << romPath);

        romFile.close();

        return {};
    }

    return {std::istreambuf_iterator(romFile), {}};
}
