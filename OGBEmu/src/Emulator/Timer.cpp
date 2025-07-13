#include "Timer.h"

#include "Core/Logger.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"

Timer::Timer(IoRegisters* ioRegisters)
    : _ioRegisters(ioRegisters), _dividerCounter(0), _timerCounter(0)
{
    Reset();
    LOG("Timer initialized");
}

void Timer::Update(int cycles)
{
    // Update DIV register (increments every 256 CPU cycles)
    _dividerCounter += cycles;
    while (_dividerCounter >= 256)
    {
        _dividerCounter -= 256;
        byte div = _ioRegisters->Read(AddressConstants::TimerDivider);
        _ioRegisters->Write(AddressConstants::TimerDivider, div + 1);
    }
    
    // Update TIMA register if timer is enabled
    if (IsTimerEnabled())
    {
        _timerCounter += cycles;
        int timerSpeed = GetTimerSpeed();
        
        while (_timerCounter >= timerSpeed)
        {
            _timerCounter -= timerSpeed;
            byte tima = _ioRegisters->Read(AddressConstants::TimerCounter);
            
            if (tima == 0xFF)
            {
                // Timer overflow
                HandleTimerOverflow();
                // After overflow, check if TMA is also 0xFF to prevent infinite loops
                byte tma = _ioRegisters->Read(AddressConstants::TimerModulo);
                if (tma == 0xFF) {
                    // If TMA is 0xFF, TIMA will immediately overflow again
                    // Break to prevent infinite loop
                    break;
                }
            }
            else
            {
                _ioRegisters->Write(AddressConstants::TimerCounter, tima + 1);
            }
        }
    }
}

void Timer::Reset()
{
    _dividerCounter = 0;
    _timerCounter = 0;
    
    // Initialize timer registers
    _ioRegisters->Write(AddressConstants::TimerDivider, 0x00);
    _ioRegisters->Write(AddressConstants::TimerCounter, 0x00);
    _ioRegisters->Write(AddressConstants::TimerModulo, 0x00);
    _ioRegisters->Write(AddressConstants::TimerControl, 0x00);
}

void Timer::HandleTimerOverflow()
{
    // Reset TIMA to TMA value
    byte tma = _ioRegisters->Read(AddressConstants::TimerModulo);
    _ioRegisters->Write(AddressConstants::TimerCounter, tma);
    
    // Request timer interrupt
    byte interruptFlag = _ioRegisters->Read(AddressConstants::InterruptFlag);
    interruptFlag |= 0x04; // Set timer interrupt flag (bit 2)
    _ioRegisters->Write(AddressConstants::InterruptFlag, interruptFlag);
}

int Timer::GetTimerSpeed() const
{
    byte tac = _ioRegisters->Read(AddressConstants::TimerControl);
    return TIMER_SPEEDS[tac & TIMER_SPEED_MASK];
}

bool Timer::IsTimerEnabled() const
{
    byte tac = _ioRegisters->Read(AddressConstants::TimerControl);
    return (tac & TIMER_ENABLE_BIT) != 0;
}