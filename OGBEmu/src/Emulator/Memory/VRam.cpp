#include "VRam.h"

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"

VRam::VRam() : _bytes(AddressConstants::EndVRamAddress - AddressConstants::StartVRamAddress + 1)
{
}

byte VRam::Read(const word busAddress)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid VRam read, address " << busAddress);
        return Read(0);
    }

    return _bytes[internalAddress];
}

void VRam::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_bytes.size())
    {
        LOG("Invalid VRam write, address " << busAddress);
        return;
    }

    _bytes[internalAddress] = data;
}

word VRam::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartVRamAddress;
}