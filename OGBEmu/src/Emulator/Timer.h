#pragma once

#include "Core/Definitions.h"

class IoRegisters;

class Timer
{
public:
    Timer(IoRegisters* ioRegisters);
    
    // Update timer with given CPU cycles
    void Update(int cycles);
    
    // Reset timer state
    void Reset();
    
private:
    IoRegisters* _ioRegisters;
    
    // Internal counters
    int _dividerCounter;    // Counts cycles for DIV register (increments every 256 cycles)
    int _timerCounter;      // Counts cycles for TIMA register
    
    // Timer control register bits
    static constexpr byte TIMER_ENABLE_BIT = 0x04;
    static constexpr byte TIMER_SPEED_MASK = 0x03;
    
    // Timer speeds (in CPU cycles)
    static constexpr int TIMER_SPEEDS[4] = {
        1024,  // 00: 4096 Hz   (CPU/1024)
        16,    // 01: 262144 Hz (CPU/16)
        64,    // 10: 65536 Hz  (CPU/64)
        256    // 11: 16384 Hz  (CPU/256)
    };
    
    // Handle timer overflow
    void HandleTimerOverflow();
    
    // Get current timer speed from TAC register
    int GetTimerSpeed() const;
    
    // Check if timer is enabled
    bool IsTimerEnabled() const;
};