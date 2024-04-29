#include "IoRegisters.h"

#include "AddressConstants.h"
#include "Core/Logger.h"

IoRegisters::IoRegisters() : _registers(AddressConstants::EndIoRegistersAddress - AddressConstants::StartIoRegistersAddress + 1)
{
}

byte IoRegisters::Read(const word busAddress) const
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_registers.size())
    {
        LOG("Invalid IO Register read, address " << busAddress);
        return 0;
    }

    return _registers[internalAddress];
}

void IoRegisters::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_registers.size())
    {
        LOG("Invalid IO Register write, address " << busAddress);
        return;
    }

    _registers[internalAddress] = data;
}

word IoRegisters::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartIoRegistersAddress;
}
