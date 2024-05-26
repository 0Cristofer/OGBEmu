#include "HRam.h"

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

HRam::HRam() : _bytes(AddressConstants::EndHRamAddress - AddressConstants::StartHRamAddress + 1)
{
}

byte HRam::Read(const word busAddress)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid HRam read, address " << busAddress);
        return Read(0);
    }

    return _bytes[internalAddress];
}

void HRam::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid HRam write, address " << busAddress);
        return;
    }

    _bytes[internalAddress] = data;
}

word HRam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartHRamAddress;
}