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
    byte ReadAtPcInc();
    byte ExecuteOpcode(Opcode opcode);
    byte ExecuteHighFunction(Opcode opcode);
    byte ExecuteLowFunction(Opcode opcode);
    inline word ReadImm16AtPc();

    byte Ld8(byte& target, byte source);
    byte Ld16(word& target, word source);
    byte LdHlSpE8();
    byte Halt();
    byte Add();
    byte Adc();
    byte Sub();
    byte Sbc();
    byte And();
    byte Xor(byte val);
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
    
    static bool IsPowerOfTwo(const unsigned int value) { return value != 0 && (value & value - 1) == 0; }

    Bus* _bus;

    union RegisterSp
    {
        word reg;
        struct
        {
            byte lo;
            byte hi;  
        };
    };

    union RegisterF
    {
        byte reg;

        struct
        {
            byte nu:4;
            byte c:1;
            byte h:1;
            byte n:1;
            byte z:1;
        };
    };
    
    union Registers
    {
        struct
        {
            word bc;
            word de;
            word hl;
            word fa;
        };
        word registers16[4];

        struct 
        {
            byte b;
            byte c;
            byte d;
            byte e;
            byte h;
            byte l;
            RegisterF f;
            byte a;
        };
        byte registers8[8];
    };

    Registers _registers;
    word _registerPc;
    RegisterSp _registerSp;

    unsigned int _framesPerSecond;
    double _frameTimeSeconds;
    double _maxCyclesPerFrame;
};
