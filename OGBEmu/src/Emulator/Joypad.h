#pragma once

#include "Core/Definitions.h"

class IoRegisters;

class Joypad
{
public:
    // Button types
    enum Button
    {
        Right  = 0,
        Left   = 1,
        Up     = 2,
        Down   = 3,
        A      = 4,
        B      = 5,
        Select = 6,
        Start  = 7
    };
    
    Joypad(IoRegisters* ioRegisters);
    
    // Set button state (true = pressed, false = released)
    void SetButtonState(Button button, bool pressed);
    
    // Update joypad register based on current state
    void Update();
    
    // Reset all button states
    void Reset();
    
private:
    IoRegisters* _ioRegisters;
    
    // Button states (true = pressed)
    bool _buttonStates[8];
    
    // Joypad register bits
    static constexpr byte SELECT_BUTTON_KEYS = 0x20;  // Bit 5: Select button keys
    static constexpr byte SELECT_DIRECTION_KEYS = 0x10; // Bit 4: Select direction keys
    
    // Update the P1 register based on current selection and button states
    void UpdateP1Register();
};