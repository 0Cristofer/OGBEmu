#include "Core/Logger.h"
#include "Core/Utils.h"

#include "Emulator/Device.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Cartridge.h"

namespace
{
    constexpr int FramesPerSecond = 128;
}

Cartridge ReadCartridge(const std::string& romPath)
{
    LOG("Cartridge rom path: " + romPath);

    std::vector<byte> romBytes = Utils::ReadBinaryFile(romPath);

    return Cartridge(std::move(romBytes));
}

BootRom ReadBootRom(const std::string& bootRomPath)
{
    LOG("Boot rom path: " + bootRomPath);

    std::vector<byte> bootRomBytes = Utils::ReadBinaryFile(bootRomPath);

    return BootRom(std::move(bootRomBytes));
}

int main(const int argc, char* argv[])
{
    if (argc != 3)
    {
        DEBUGBREAKLOG("Wrong number of program arguments, usage: OGBEmu bootRom.bin romPath.gb");
        return 0;
    }

    const std::string bootRomPath(argv[1]);
    BootRom bootRom = ReadBootRom(bootRomPath);
    if (!bootRom.IsValid())
    {
        DEBUGBREAKLOG("Invalid boot ROM, check path and file size. Only " << GbConstants::BootRomSize <<"-byte ROMs are accepted.");
        return 0;
    }

    const std::string romPath(argv[2]);
    Cartridge cartridge = ReadCartridge(romPath);
    if (!cartridge.IsValid())
    {
        DEBUGBREAKLOG("Invalid cartridge ROM, check path and file size. Cartridge ROM sizes are minimun " << GbConstants::MinCartridgeRomSize << " bytes.");
        return 0;
    }

    LOG("");
    LOG("Starting up device");
    Device device(std::move(bootRom), std::move(cartridge), FramesPerSecond);

    LOG("Running");
    device.Run();

    LOG("Finished");
    return 0;
}
