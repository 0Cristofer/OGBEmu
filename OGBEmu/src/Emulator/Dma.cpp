#include "Dma.h"

#include "Core/Logger.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"

Dma::Dma(Bus* bus)
    : _bus(bus), _isActive(false), _sourceAddress(0), _bytesTransferred(0)
{
    LOG("DMA initialized");
}

void Dma::Update()
{
    if (!_isActive)
    {
        return;
    }
    
    // Transfer one byte per update (simulating the 160 cycle transfer)
    if (_bytesTransferred < BYTES_TO_TRANSFER)
    {
        // Read from source address
        byte data = _bus->Read(_sourceAddress + _bytesTransferred);
        
        // Write to OAM (0xFE00 + offset)
        word oamAddress = AddressConstants::StartOamAddress + _bytesTransferred;
        _bus->Write(oamAddress, data);
        
        _bytesTransferred++;
        
        // Check if transfer is complete
        if (_bytesTransferred >= BYTES_TO_TRANSFER)
        {
            _isActive = false;
        }
    }
}

void Dma::StartTransfer(byte sourceHighByte)
{
    // DMA source address is XX00 where XX is the written value
    _sourceAddress = sourceHighByte << 8;
    _bytesTransferred = 0;
    _isActive = true;
    
    // During DMA, only HRAM is accessible
    // This is handled by the Bus during reads/writes
}

void Dma::Reset()
{
    _isActive = false;
    _sourceAddress = 0;
    _bytesTransferred = 0;
}