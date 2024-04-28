#include "Cartridge.h"
#include "Device.h"
#include "Logger.h"
#include "Utils.h"

namespace
{
    constexpr int FramesPerSecond = 128;
}

int main(const int argc, char* argv[])
{
    if (argc != 2)
    {
        Logger::Log("Not enough program arguments, usage: OGBEmu romPath.gb");
        return 0;
    }

    const std::string romPath(argv[1]);
    Logger::Log("Rom path: " + romPath);

    std::vector<unsigned char> romBytes = Utils::ReadBinaryFile(romPath);

    Cartridge cartridge(std::move(romBytes));
    if (!cartridge.IsValid())
    {
        return 0;
    }

    Device device(std::move(cartridge), FramesPerSecond);

    device.Run();

    return 0;
}
