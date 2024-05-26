#include "WRam.h"

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

WRam::WRam() : _bytes(AddressConstants::EndWRamAddress - AddressConstants::StartWRamAddress + 1)
{
}

byte WRam::Read(const word busAddress)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid WRam read, address " << busAddress);
        return Read(0);
    }

    return _bytes[internalAddress];
}

void WRam::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid WRam write, address " << busAddress);
        return;
    }

    _bytes[internalAddress] = data;
}

word WRam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartWRamAddress;
}
