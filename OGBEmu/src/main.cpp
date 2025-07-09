#include "Core/Logger.h"
#include "Core/Utils.h"

#include "Emulator/Device.h"
#include "Emulator/Screen.h"

#include <string>

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
    if (argc < 3 || argc > 4)
    {
        DEBUGBREAKLOG("Wrong number of program arguments, usage: OGBEmu bootRom.bin romPath.gb [timeout (0=infinite)]");
        return 0;
    }

    const std::string bootRomPath(argv[1]);
    const std::vector<byte> bootRomBytes = ReadBootRom(bootRomPath);

    const std::string romPath(argv[2]);
    const std::vector<byte> cartridgeBytes = ReadCartridge(romPath);

    double timeoutSeconds = 0.0;  // Default: run indefinitely
    if (argc == 4)
    {
        try
        {
            timeoutSeconds = std::stod(argv[3]);
            if (timeoutSeconds < 0)
            {
                DEBUGBREAKLOG("Timeout must be non-negative, got: " << timeoutSeconds);
                return 0;
            }
        }
        catch (const std::exception& e)
        {
            DEBUGBREAKLOG("Invalid timeout value: " << argv[3] << ", error: " << e.what());
            return 0;
        }
    }

    LOG("");
    LOG("Starting up device");
    Screen screen;
    Device device(bootRomBytes, cartridgeBytes, FramesPerSecond, timeoutSeconds, &screen);

    if (!device.IsValid())
    {
        LOG("Invalid device, quitting");
        return 0;
    }

    device.Run();

    return 0;
}
