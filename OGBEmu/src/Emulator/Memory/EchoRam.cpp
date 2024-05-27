#include "EchoRam.h"

#include <format>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

EchoRam::EchoRam() : _bytes(AddressConstants::EndEchoRamAddress - AddressConstants::StartEchoRamAddress + 1)
{
}

byte EchoRam::Read(const word busAddress) const
{
    DEBUGBREAKLOG("Invalid EchoRam read, address " << std::format("{:x}", busAddress));

    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid EchoRam read, address " << std::format("{:x}", busAddress));
        return 0;
    }

    return _bytes[internalAddress];
}

void EchoRam::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        DEBUGBREAKLOG("Invalid EchoRam write, address " << std::format("{:x}", busAddress));
        return;
    }

    _bytes[internalAddress] = data;
}

word EchoRam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartEchoRamAddress;
}
