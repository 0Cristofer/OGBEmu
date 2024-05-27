#include "HRam.h"

#include <format>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

HRam::HRam() : _bytes(AddressConstants::EndHRamAddress - AddressConstants::StartHRamAddress + 1)
{
}

byte HRam::Read(const word busAddress) const
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid HRam read, address " << std::format("{:x}", busAddress));
        return 0;
    }

    return _bytes[internalAddress];
}

void HRam::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid HRam write, address " << std::format("{:x}", busAddress));
        return;
    }

    _bytes[internalAddress] = data;
}

word HRam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartHRamAddress;
}