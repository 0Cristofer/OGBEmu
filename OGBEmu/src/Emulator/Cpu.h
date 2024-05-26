#pragma once

#include "Core/Definitions.h"

#include "Emulator/Opcode.h"

class Bus;

class Cpu
{
public:
    explicit Cpu(Bus* bus);

    byte Update();

    static constexpr unsigned int CpuClock = 4194304;

private:
    union Register16;
    
    Opcode FetchNextOpcode();
    void UpdateIme();
    void HandleInterrupts();

    [[nodiscard]] byte ReadAtPcInc();
    [[nodiscard]] byte ReadBus(word address);
    [[nodiscard]] byte ReadAtSp() const;
    void WriteBus(word address, byte data);
    void WriteAtSp(byte data) const;
    inline word ReadImm16AtPc();

    void ExecuteOpcode(Opcode opcode);
    void ExecuteHighFunction(Opcode opcode);
    void ExecuteLowFunction(Opcode opcode);
    void ExecutePrefix();

    // Most ALU instructions were based on https://github.com/mgba-emu/mgba/blob/master/src/sm83/isa-sm83.c
    void Ld8R(byte targetIndex, byte sourceIndex);
    void Ld8Imm(byte targetIndex);
    void Ld8TaImm(word address);
    void Ld8Sa(byte targetIndex, word sourceAddress);
    void Ld8Ta(word targetAddress, byte sourceIndex);
    void Ld16Imm(byte targetIndex);
    void LdSpTImm();
    void LdSpS(word val);
    void LdImmTaSp();
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
    static void Nop();
    void Stop();
    void Jr(byte test);
    void Ret();
    void RetTest(byte test);
    void AddSp();
    void Jp(word address);
    void JpTest(byte test, word address);
    void JpHl();
    void Inc8(byte& target);
    void Inc8Add(word address);
    void Inc16(word& target);
    void Di();
    void Ei();
    void Call(word address);
    void CallTest(byte test, word address);
    void Dec8(byte& target);
    void Dec8Add(word address);
    void Dec16(word& target);
    void Add16(word& target);
    void Push(Register16 register16Data);
    void Pop(Register16& register16Target);
    void Rlca();
    void Rla();
    void Daa();
    void Scf();
    void Rst(word address);
    void Reti();
    void Rrca();
    void Rra();
    void Cpl();
    void Ccf();
    void Rlc(byte& reg);
    void Rrc(byte& reg);
    void Rl(byte& reg);
    void Rr(byte& reg);
    void Sla(byte& reg);
    void Sra(byte& reg);
    void Swap(byte& reg);
    void Srl(byte& reg);
    void Bit(byte testBit, byte testR8);
    static void Res(byte testBit, byte& testR8);
    static void Set(byte testBit, byte& testR8);
    
    static byte ConvertReg8Index(const byte opcodeRegIndex)
    {
        return opcodeRegIndex == 0x7 ? opcodeRegIndex : opcodeRegIndex / 2 * 2 + !(opcodeRegIndex % 2);
    }

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
            byte nu : 4;
            byte c : 1;
            byte h : 1;
            byte n : 1;
            byte z : 1;
        };
    };

    union Registers
    {
        struct
        {
            Register16 bc;
            Register16 de;
            Register16 hl;
            Register16 af;
        };

        Register16 registers16[4];

        struct
        {
            byte c;
            byte b;
            byte e;
            byte d;
            byte l;
            byte h;
            RegisterF f;
            byte a;
        };

        byte registers8[8];
    };

    Registers _registers;
    Register16 _registerSp;
    Register16 _registerPc;

    Bus* _bus;

    byte _cyclesThisInstruction = 0;
    byte _ime;
    byte _halted;
    bool _eiRequested;
};
