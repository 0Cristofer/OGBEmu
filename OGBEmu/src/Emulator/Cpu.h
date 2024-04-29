#pragma once

#include "Opcode.h"
#include "Core/Definitions.h"

class Bus;

enum FlagBit : byte
{
    Zero = 7,
    Subtraction = 6,
    HalfCarry = 5,
    Carry = 4,
};

class Cpu
{
public:
    explicit Cpu(Bus* bus, unsigned int framesPerSecond);

    void Run();

private:
    unsigned int DoFrame();
    void WaitForNextFrame(double frameTimeSeconds) const;

    byte ExecuteNextInstruction();
    Opcode FetchNextOpcode();
    byte ExecuteOpcode(Opcode opcode);
    byte ExecuteRowFunction(Opcode opcode);
    byte ExecuteColumnFunction(Opcode opcode);

    byte Ld();
    byte Halt();
    byte Add();
    byte Adc();
    byte Sub();
    byte Sbc();
    byte And();
    byte Xor();
    byte Or();
    byte Cp();

    byte Nop();
    byte Stop();
    byte Jr();
    byte Ret();
    byte Pop();
    byte Jp();
    byte Inc();
    byte Di();
    byte Call();
    byte Dec();
    byte Push();
    byte Rlca();
    byte Rla();
    byte Daa();
    byte Scf();
    byte Rst();
    byte Reti();
    byte Prefix();
    byte Rrca();
    byte Rra();
    byte Cpl();
    byte Ccf();

    void SetFlag(FlagBit flagBit, byte val);
    [[nodiscard]] byte GetFlag(FlagBit flagBit) const;
    
    static bool IsPowerOfTwo(const unsigned int value) { return value != 0 && (value & value - 1) == 0; }

    Bus* _bus;

    union Register
    {
        word reg;
        struct
        {
            byte lo;
            byte hi;
        };
    };

    Register _registerPc;
    
    Register _registerAF;
    Register _registerBC;
    Register _registerDE;
    Register _registerHL;

    Register _registerSp;

    unsigned int _framesPerSecond;
    double _frameTimeSeconds;
    double _maxCyclesPerFrame;
};
