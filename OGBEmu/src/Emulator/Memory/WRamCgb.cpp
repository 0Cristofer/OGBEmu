#include "WRamCgb.h"

#include <format>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

WRamCgb::WRamCgb() : _bytes(AddressConstants::EndWRamCgbAddress - AddressConstants::StartWRamCgbAddress + 1)
{
}

byte WRamCgb::Read(const word busAddress) const
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid WRamCgb read, address " << std::format("{:x}", busAddress));
        return 0;
    }

    return _bytes[internalAddress];
}

void WRamCgb::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid WRamCgb write, address " << std::format("{:x}", busAddress));
        return;
    }

    _bytes[internalAddress] = data;
}

word WRamCgb::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartWRamCgbAddress;
}
