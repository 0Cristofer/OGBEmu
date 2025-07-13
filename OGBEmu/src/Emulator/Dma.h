#pragma once

#include "Core/Definitions.h"

class Bus;
class IoRegisters;

class Dma
{
public:
    Dma(Bus* bus);
    
    // Update DMA transfer (returns cycles consumed)
    int Update();
    
    // Start DMA transfer when 0xFF46 is written
    void StartTransfer(byte sourceHighByte);
    
    // Check if DMA transfer is active
    bool IsActive() const { return _isActive; }
    
    // Reset DMA state
    void Reset();
    
private:
    Bus* _bus;
    
    // DMA state
    bool _isActive;
    word _sourceAddress;
    int _bytesTransferred;
    
    // DMA takes 160 machine cycles (640 CPU cycles)
    static constexpr int DMA_CYCLES = 160 * 4;
    static constexpr int BYTES_TO_TRANSFER = 160; // 40 sprites * 4 bytes each
};