#include "EchoRam.h"

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

EchoRam::EchoRam() : _bytes(AddressConstants::EndEchoRamAddress - AddressConstants::StartEchoRamAddress + 1)
{
}

byte& EchoRam::ReadRef(const word busAddress)
{
    LOG("Invalid EchoRam read, address " << busAddress);

    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        return ReadRef(0);
    }

    return _bytes[internalAddress];
}

byte EchoRam::Read(const word busAddress)
{
    return ReadRef(busAddress);
}

void EchoRam::Write(const word busAddress, const byte data)
{
    LOG("Invalid EchoRam write, address " << busAddress);

    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        return;
    }

    _bytes[internalAddress] = data;
}

word EchoRam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartEchoRamAddress;
}
