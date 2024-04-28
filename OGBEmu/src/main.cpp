#include "Core/Logger.h"
#include "Core/Utils.h"
#include "Emulator/BootRom.h"

#include "Emulator/Cartridge.h"
#include "Emulator/Device.h"

namespace
{
    constexpr int FramesPerSecond = 128;
}

Cartridge ReadCartridge(const std::string& romPath)
{
    Logger::Log("Cartridge rom path: " + romPath);

    std::vector<unsigned char> romBytes = Utils::ReadBinaryFile(romPath);

    return Cartridge(std::move(romBytes));
}

BootRom ReadBootRom(const std::string& bootRomPath)
{
    Logger::Log("Boot rom path: " + bootRomPath);

    std::vector<unsigned char> bootRomBytes = Utils::ReadBinaryFile(bootRomPath);

    return BootRom(std::move(bootRomBytes));
}

int main(const int argc, char* argv[])
{
    if (argc != 3)
    {
        Logger::Log("Not enough program arguments, usage: OGBEmu bootRom.bin romPath.gb");
        return 0;
    }

    const std::string bootRomPath(argv[1]);
    BootRom bootRom = ReadBootRom(bootRomPath);
    if (!bootRom.IsValid())
    {
        LOG("Invalid boot ROM, check path and file size. Only " << GbConstants::BootRomSize <<"-byte ROMs are accepted.");
        return 0;
    }

    const std::string romPath(argv[2]);
    Cartridge cartridge = ReadCartridge(romPath);
    if (!cartridge.IsValid())
    {
        LOG("Invalid cartridge ROM, check path and file size. Cartridge ROM sizes are minimun " << GbConstants::MinCartridgeRomSize << " bytes.");
        return 0;
    }

    Device device(std::move(bootRom), std::move(cartridge), FramesPerSecond);

    device.Run();

    return 0;
}
