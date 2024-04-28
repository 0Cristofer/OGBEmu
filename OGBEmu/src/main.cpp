#include <fstream>
#include <vector>

#include "Cartridge.h"
#include "Logger.h"

int main(const int argc, char** argv)
{
    if (argc != 2)
    {
        Logger::Log("Not enough program arguments, usage: OGBEmu romPath.gb");
        return 0;
    }

    const std::string romPath(argv[1]);

    Logger::Log("Rom path: " + romPath);

    const Cartridge cartridge(romPath);

    if (!cartridge.IsValid())
    {
        return 0;
    }

    return 0;
}
