#include "Cartridge.h"

#include <utility>

Cartridge::Cartridge(std::vector<byte> romBytes) : _rom(std::move(romBytes))
{
    if (!IsValid())
    {
        return;
    }

    _cartridgeType = static_cast<CartridgeType>(_rom[GbConstants::CartridgeTypeAddress]);
}
