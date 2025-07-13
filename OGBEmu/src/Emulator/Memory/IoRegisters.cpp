#include "IoRegisters.h"

#include <format>

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Apu.h"

IoRegisters::IoRegisters() : _registers(AddressConstants::EndIoRegistersAddress - AddressConstants::StartIoRegistersAddress + 1), _apu(nullptr)
{
}

byte IoRegisters::Read(const word busAddress) const
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_registers.size())
    {
        ERROR("Invalid IO Register read, address " << std::format("{:x}", busAddress));
        return 0;
    }

    return _registers[internalAddress];
}

void IoRegisters::Write(const word busAddress, const byte data)
{
    const word internalAddress = TranslateAddress(busAddress);

    if (internalAddress < 0 || internalAddress >=_registers.size())
    {
        ERROR("Invalid IO Register write, address " << std::format("{:x}", busAddress));
        return;
    }

    _registers[internalAddress] = data;
    
    // Notify APU of audio register writes
    if (_apu && busAddress >= 0xFF10 && busAddress <= 0xFF3F)
    {
        _apu->WriteRegister(busAddress, data);
    }
}

word IoRegisters::TranslateAddress(const word busAddress)
{
    return busAddress - AddressConstants::StartIoRegistersAddress;
}
