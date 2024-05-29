#include "Core/Logger.h"
#include "Core/Utils.h"

#include "Emulator/Device.h"

namespace
{
    constexpr int FramesPerSecond = 128;
}

std::vector<byte> ReadCartridge(const std::string& romPath)
{
    LOG("Cartridge rom path: " + romPath);

    std::vector<byte> romBytes = Utils::ReadBinaryFile(romPath);

    return romBytes;
}

std::vector<byte> ReadBootRom(const std::string& bootRomPath)
{
    LOG("Boot rom path: " + bootRomPath);

    std::vector<byte> bootRomBytes = Utils::ReadBinaryFile(bootRomPath);

    return bootRomBytes;
}

int main(const int argc, char* argv[])
{
    if (argc != 3)
    {
        DEBUGBREAKLOG("Wrong number of program arguments, usage: OGBEmu bootRom.bin romPath.gb");
        return 0;
    }

    const std::string bootRomPath(argv[1]);
    const std::vector<byte> bootRomBytes = ReadBootRom(bootRomPath);

    const std::string romPath(argv[2]);
    const std::vector<byte> cartridgeBytes = ReadCartridge(romPath);

    LOG("");
    LOG("Starting up device");
    Device device(bootRomBytes, cartridgeBytes, FramesPerSecond);

    if (!device.IsValid())
    {
        LOG("Invalid device, quitting");
        return 0;
    }

    LOG("Running");
    device.Run();

    LOG("Finished");
    return 0;
}
