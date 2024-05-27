#include "Oam.h"

#include <format>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

Oam::Oam() : _bytes(AddressConstants::EndOamAddress - AddressConstants::StartOamAddress + 1)
{
}

byte Oam::Read(const word busAddress) const
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid Oam read, address " << std::format("{:x}", busAddress));
        return 0;
    }

    return _bytes[internalAddress];
}

void Oam::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid Oam write, address " << std::format("{:x}", busAddress));
        return;
    }

    _bytes[internalAddress] = data;
}

word Oam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartOamAddress;
}