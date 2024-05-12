#include "Oam.h"

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

Oam::Oam() : _bytes(AddressConstants::EndOamAddress - AddressConstants::StartOamAddress + 1)
{
}

byte& Oam::ReadRef(const word busAddress)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid Oam read, address " << busAddress);
        return ReadRef(0);
    }

    return _bytes[internalAddress];
}

byte Oam::Read(const word busAddress)
{
    return ReadRef(busAddress);
}

void Oam::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid Oam write, address " << busAddress);
        return;
    }

    _bytes[internalAddress] = data;
}

word Oam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartOamAddress;
}