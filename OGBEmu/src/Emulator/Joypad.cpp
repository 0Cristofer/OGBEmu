#include "Joypad.h"

#include "Core/Logger.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/AddressConstants.h"

Joypad::Joypad(IoRegisters* ioRegisters)
    : _ioRegisters(ioRegisters)
{
    Reset();
    LOG("Joypad initialized");
}

void Joypad::SetButtonState(Button button, bool pressed)
{
    bool previousState = _buttonStates[button];
    _buttonStates[button] = pressed;
    
    // If any button was pressed (transition from unpressed to pressed), request interrupt
    if (!previousState && pressed)
    {
        byte interruptFlag = _ioRegisters->Read(AddressConstants::InterruptFlag);
        interruptFlag |= 0x10; // Set joypad interrupt flag (bit 4)
        _ioRegisters->Write(AddressConstants::InterruptFlag, interruptFlag);
    }
    
    // Update P1 register
    UpdateP1Register();
}

void Joypad::Update()
{
    // Update P1 register based on current state
    UpdateP1Register();
}

void Joypad::Reset()
{
    // All buttons unpressed
    for (int i = 0; i < 8; i++)
    {
        _buttonStates[i] = false;
    }
    
    // Initialize P1 register to 0xFF (all buttons unpressed)
    _ioRegisters->Write(AddressConstants::Joypad, 0xFF);
}

void Joypad::UpdateP1Register()
{
    byte p1 = _ioRegisters->Read(AddressConstants::Joypad);
    
    // Keep only bits 4-5 from current value (selection bits are writable)
    p1 &= 0x30;
    
    // Set unused bits to 1
    p1 |= 0xC0;
    
    // Check which keys are selected
    bool selectButtons = (p1 & SELECT_BUTTON_KEYS) == 0;
    bool selectDirections = (p1 & SELECT_DIRECTION_KEYS) == 0;
    
    // Set bits 0-3 based on selected keys (0 = pressed, 1 = not pressed)
    if (selectDirections)
    {
        if (_buttonStates[Right]) p1 &= ~0x01;
        else p1 |= 0x01;
        
        if (_buttonStates[Left]) p1 &= ~0x02;
        else p1 |= 0x02;
        
        if (_buttonStates[Up]) p1 &= ~0x04;
        else p1 |= 0x04;
        
        if (_buttonStates[Down]) p1 &= ~0x08;
        else p1 |= 0x08;
    }
    else if (selectButtons)
    {
        if (_buttonStates[A]) p1 &= ~0x01;
        else p1 |= 0x01;
        
        if (_buttonStates[B]) p1 &= ~0x02;
        else p1 |= 0x02;
        
        if (_buttonStates[Select]) p1 &= ~0x04;
        else p1 |= 0x04;
        
        if (_buttonStates[Start]) p1 &= ~0x08;
        else p1 |= 0x08;
    }
    else
    {
        // No keys selected, all bits high
        p1 |= 0x0F;
    }
    
    _ioRegisters->Write(AddressConstants::Joypad, p1);
}