#pragma once

#include "Core/Definitions.h"

#include "Emulator/Opcode.h"

class Bus;

class Cpu
{
public:
    explicit Cpu(Bus* bus);

    byte ExecuteNextInstruction();

    static constexpr unsigned int CpuClock = 4194304;

private:
    Opcode FetchNextOpcode();

    [[nodiscard]] byte ReadAtPcInc();
    [[nodiscard]] byte& ReadBusRef(word address);
    [[nodiscard]] byte ReadBus(word address);
    inline word ReadImm16AtPc();

    void ExecuteOpcode(Opcode opcode);
    void ExecuteHighFunction(Opcode opcode);
    void ExecuteLowFunction(Opcode opcode);
    void ExecutePrefix();
    
    static void Ld8(byte& target, byte source);
    static void Ld16(word& target, word source);
    void LdHlSpE8();
    void Halt();
    void Add(byte val);
    void Adc(byte val);
    void Sub(byte val);
    void Sbc(byte val);
    void And(byte val);
    void Xor(byte val);
    void Or(byte val);
    void Cp(byte val);

    void Nop();
    void Stop();
    void Jr(byte test);
    void Ret(byte test);
    void Pop();
    void AddSp();
    void Jp();
    void Inc8(byte& target);
    static void Inc16(word& target);
    void Di();
    void Ei();
    void Call(byte test, word address);
    void Dec(byte& target);
    void Dec16(word& target);
    void Add16(word& target);
    void Push();
    void Rlca();
    void Rla();
    void Daa();
    void Scf();
    void Rst();
    void Reti();
    void Rrca();
    void Rra();
    void Cpl();
    void Ccf();
    void Rlc();
    void Rrc();
    void Rl();
    void Rr();
    void Sla();
    void Sra();
    void Swap();
    void Srl();
    void Bit(byte testBit, byte testR8);
    void Res(byte testBit, byte testR8);
    void Set(byte testBit, byte testR8);
    
    enum FlagBit : byte
    {
        Zero = 7,
        Subtraction = 6,
        HalfCarry = 5,
        Carry = 4,
    };

    union Register16
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
            byte z : 1;
            byte n : 1;
            byte h : 1;
            byte c : 1;
            byte nu : 4;
        };
    };

    union Registers
    {
        struct
        {
            Register16 bc;
            Register16 de;
            Register16 hl;
            Register16 fa;
        };

        Register16 registers16[4];

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
    Register16 _registerSp;

    Bus* _bus;

    byte _cyclesThisInstruction = 0;
};
