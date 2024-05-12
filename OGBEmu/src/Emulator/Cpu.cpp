#include "Cpu.h"

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/Bus.h"

Cpu::Cpu(Bus* bus) : _registers(), _registerSp(), _bus(bus)
{
    // This is the only hardware initialization needed, everything else is done by the boot rom
    _registerPc = 0;
    _bus->Write(AddressConstants::BootRomBank, 0);
}

byte Cpu::ExecuteNextInstruction()
{
    _cyclesThisInstruction = 0;
    
    const Opcode opcode = FetchNextOpcode();
    ExecuteOpcode(opcode);

    return _cyclesThisInstruction;
}

Opcode Cpu::FetchNextOpcode()
{
    Opcode opcode;
    opcode.code = ReadAtPcInc();

    return opcode;
}

byte Cpu::ReadAtPcInc()
{
    return ReadBus(_registerPc++);
}

byte& Cpu::ReadBusRef(const word address)
{
    _cyclesThisInstruction += 4;
    return _bus->ReadRef(address);
}

byte Cpu::ReadBus(const word address)
{
    _cyclesThisInstruction += 4;
    return _bus->Read(address);
}

word Cpu::ReadImm16AtPc()
{
    const byte firstByte = ReadAtPcInc();
    const word imm = static_cast<word>(firstByte) + static_cast<word>(ReadAtPcInc() << 8);

    return imm;
}

void Cpu::ExecuteOpcode(const Opcode opcode)
{
    if (opcode.high > 0x3 && opcode.high < 0xC)
        return ExecuteHighFunction(opcode);
    return ExecuteLowFunction(opcode);
}

void Cpu::ExecuteHighFunction(const Opcode opcode)
{
    if (opcode.row5 > 010 && opcode.row5 < 020)
    {
        if (opcode.row5 == 016 && opcode.column3 == 06)
            return Halt();

        const byte targetIndex = opcode.row5 - 010;
        const byte sourceIndex = opcode.column3;

        if (targetIndex == 6)
            return Ld8(ReadBusRef(_registers.hl.reg), _registers.registers8[sourceIndex]);
        if (sourceIndex == 6)
            return Ld8(_registers.registers8[targetIndex], ReadBusRef(_registers.hl.reg));
        return Ld8(_registers.registers8[targetIndex], _registers.registers8[sourceIndex]);
    }

    if (opcode.row5 == 020)
    {
        if (opcode.column3 == 06)
            return Add(ReadBus(_registers.hl.reg));
        return Add(_registers.registers8[opcode.column3]);
    }

    if (opcode.row5 == 021)
    {
        if (opcode.column3 == 06)
            return Adc(ReadBus(_registers.hl.reg));
        return Adc(_registers.registers8[opcode.column3]);
    }

    if (opcode.row5 == 022)
    {
        if (opcode.column3 == 06)
            return Sub(ReadBus(_registers.hl.reg));
        return Sub(_registers.registers8[opcode.column3]);
    }

    if (opcode.row5 == 023)
    {
        if (opcode.column3 == 06)
            return Sbc(ReadBus(_registers.hl.reg));
        return Sbc(_registers.registers8[opcode.column3]);
    }

    if (opcode.row5 == 024)
    {
        if (opcode.column3 == 06)
            return And(ReadBus(_registers.hl.reg));
        return And(_registers.registers8[opcode.column3]);
    }

    if (opcode.row5 == 025)
    {
        if (opcode.column3 == 06)
            return Xor(ReadBus(_registers.hl.reg));
        return Xor(_registers.registers8[opcode.column3]);
    }

    if (opcode.row5 == 026)
    {
        if (opcode.column3 == 06)
            return Or(ReadBus(_registers.hl.reg));
        return Or(_registers.registers8[opcode.column3]);
    }

    if (opcode.row5 == 027)
    {
        if (opcode.column3 == 06)
            return Cp(ReadBus(_registers.hl.reg));
        return Cp(_registers.registers8[opcode.column3]);
    }
    
    LOG("Row function Op Code not found. Row " << static_cast<int>(opcode.high) << ", column " << static_cast<int>(
        opcode.low));
}

void Cpu::ExecuteLowFunction(const Opcode opcode)
{
    if (opcode.column3 == 00)
    {
        if (opcode.row5 == 03)
            return Jr(1);
        if (opcode.row5 < 010)
        {
            const byte flag = opcode.row5 < 06 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0 ? !flag : flag;
            
            return Jr(test);
        }

        if (opcode.row5 > 027 && opcode.row5 < 034)
        {
            const byte flag = opcode.row5 < 032 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0 ? !flag : flag;
            
            return Ret(test);
        }
    }
    
    if (opcode.low == 0x0)
    {
        if (opcode.high == 0x0)
        {
            return Nop();
        }

        if (opcode.high == 0x1)
        {
            return Stop();
        }

        if (opcode.high == 0xE || opcode.high == 0xF)
        {
            const word imm = 0xff00 + static_cast<word>(ReadAtPcInc());
            byte& valAtImm = ReadBusRef(imm);

            if (opcode.high == 0xE)
                return Ld8(valAtImm, _registers.a);
            return Ld8(_registers.a, valAtImm);
        }
    }

    if (opcode.low == 0x1)
    {
        if (opcode.high < 0x4)
        {
            Register16& target = opcode.high == 0x3 ? _registerSp : _registers.registers16[opcode.row];
            return Ld16(target.reg, ReadImm16AtPc());
        }

        if (opcode.high > 0xB)
        {
            return Pop();
        }
    }

    if (opcode.column3 == 02)
    {
        if (opcode.row5 < 010)
        {
            if (opcode.row5 % 2 == 0)
            {
                if (opcode.row == 0x2)
                    return Ld8(ReadBusRef(_registers.hl.reg++), _registers.a);
                if (opcode.row == 0x3)
                    return Ld8(ReadBusRef(_registers.hl.reg--), _registers.a);
                
                return Ld8(ReadBusRef(_registers.registers16[opcode.row].reg), _registers.a);
            }

            if (opcode.row == 0x2)
                return Ld8(_registers.a, ReadBusRef(_registers.hl.reg++));
            if (opcode.row == 0x3)
                return Ld8(_registers.a, ReadBusRef(_registers.hl.reg--));
                
            return Ld8( _registers.a, ReadBusRef(_registers.registers16[opcode.row].reg));
        }

        if (opcode.row5 > 027 && opcode.row5 < 034)
            return Jp();

        if (opcode.row5 > 033)
        {
            if (opcode.row5 % 2 == 0)
            {
                const word address = 0xff00 + static_cast<word>(_registers.c);
                if (opcode.high == 0xE)
                    return Ld8(ReadBusRef(address), _registers.a);
                if (opcode.high == 0xF)
                    return Ld8(_registers.a, ReadBus(address));
            }

            if (opcode.high == 0xE)
                return Ld8(ReadBusRef(ReadImm16AtPc()), _registers.a);
            if (opcode.high == 0xF)
                return Ld8(_registers.a, ReadBus(ReadImm16AtPc()));
        }
    }

    if (opcode.low == 0x3)
    {
        if (opcode.high < 0x3)
            return Inc16(_registers.registers16[opcode.high].reg);
        if (opcode.high == 0x4)
            return Inc16(_registerSp.reg);

        if (opcode.high == 0xC)
        {
            return Jp();
        }

        if (opcode.high == 0xF)
        {
            return Di();
        }
    }

    if (opcode.column3 == 04)
    {
        if (opcode.row5 == 06)
        {
            _cyclesThisInstruction += 4;
            return Inc8(ReadBusRef(_registers.hl.reg));
        }
        if (opcode.row5 < 010)
            return Inc8(_registers.registers8[opcode.row5]);

        if (opcode.row5 > 027 && opcode.row5 < 034)
        {
            const byte flag = opcode.row5 < 032 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0 ? !flag : flag;
            
            return Call(test, ReadImm16AtPc());
        }
    }

    if (opcode.column3 == 05)
    {
        if (opcode.row5 == 06)
        {
            _cyclesThisInstruction += 4;
            return Dec(ReadBusRef(_registers.hl.reg));
        }
        if (opcode.row5 < 010)
            return Dec(_registers.registers8[opcode.row5]);
    }

    if (opcode.low == 0x5)
    {
        if (opcode.high > 0xB)
        {
            return Push();
        }
    }

    if (opcode.column3 == 06)
    {
        if (opcode.row5 < 010)
        {
            const byte source = ReadAtPcInc();
            const byte targetRegister = opcode.row5;

            if (targetRegister == 0x6)
                return Ld8(ReadBusRef(_registers.hl.reg), source);
            return Ld8(_registers.registers8[targetRegister], source);
        }
    }

    if (opcode.column3 == 07)
    {
        if (opcode.row5 > 027)
            return Rst();
    }

    if (opcode.low == 0x6)
    {
        if (opcode.high == 0xC)
            return Add(ReadAtPcInc());
        if (opcode.high == 0xD)
            return Sub(ReadAtPcInc());
        if (opcode.high == 0xE)
            return And(ReadAtPcInc());
        if (opcode.high == 0xF)
            return Or(ReadAtPcInc());
    }

    if (opcode.low == 0x7)
    {
        if (opcode.high == 0x0)
        {
            return Rlca();
        }

        if (opcode.high == 0x1)
        {
            return Rla();
        }

        if (opcode.high == 0x2)
        {
            return Daa();
        }

        if (opcode.high == 0x3)
        {
            return Scf();
        }
    }

    if (opcode.low == 0x8)
    {
        if (opcode.high == 0x0)
        {
            const word imm = ReadImm16AtPc();

            Ld8(ReadBusRef(imm), _registerSp.hi);
            Ld8(ReadBusRef(imm + 1), _registerSp.lo);
            return;
        }

        if (opcode.high == 0xE)
            return AddSp();
        if (opcode.high == 0xF)
            return LdHlSpE8();
    }

    if (opcode.low == 0x9)
    {
        if (opcode.high < 0x3)
            return Add16(_registers.registers16[opcode.high].reg);
        if (opcode.high == 0x4)
            return Add16(_registerSp.reg);

        if (opcode.high == 0xC)
            return Ret(1);
        if (opcode.high == 0xD)
            return Reti();
        if (opcode.high == 0xE)
        {
            return Jp();
        }

        if (opcode.high == 0xF)
        {
            _cyclesThisInstruction += 4;
            
            Ld16(_registerSp.reg, _registers.hl.reg);
            return;
        }
    }

    if (opcode.low == 0xB)
    {
        if (opcode.high < 0x3)
            return Dec16(_registers.registers16[opcode.high].reg);
        if (opcode.high == 0x4)
            return Dec16(_registerSp.reg);

        if (opcode.high == 0xC)
            return ExecutePrefix();

        if (opcode.high == 0xF)
            return Ei();
    }
    
    if (opcode.low == 0xD)
    {
        if (opcode.high == 0xC)
        {
            return Call(1, ReadImm16AtPc());
        }
    }

    if (opcode.low == 0xE)
    {
        if (opcode.high == 0xC)
        {
            return Adc(ReadAtPcInc());
        }

        if (opcode.high == 0xD)
        {
            return Sbc(ReadAtPcInc());
        }

        if (opcode.high == 0xE)
        {
            Xor(ReadAtPcInc());
            return;
        }

        if (opcode.high == 0xF)
        {
            return Cp(ReadAtPcInc());
        }
    }

    if (opcode.low == 0xF)
    {
        if (opcode.high == 0x0)
        {
            return Rrca();
        }

        if (opcode.high == 0x1)
        {
            return Rra();
        }

        if (opcode.high == 0x2)
        {
            return Cpl();
        }

        if (opcode.high == 0x3)
        {
            return Ccf();
        }
    }

    LOG("Column function Op Code not found. Row " << static_cast<int>(opcode.high) << ", column " << static_cast<int>(
        opcode.low));
}

void Cpu::ExecutePrefix()
{
    const Opcode prefixOpcode = FetchNextOpcode();

    if (prefixOpcode.row5 > 07 && prefixOpcode.row5 < 020)
    {
        const byte testR8 = prefixOpcode.column3 != 06
                             ? _registers.registers8[prefixOpcode.column3]
                             : ReadBus(_registers.hl.reg);
        const byte testBit = (prefixOpcode.row5 - 010) % 010;

        Bit(testBit, testR8);
        return;
    }
    
    const byte& target = prefixOpcode.column3 != 06
                             ? _registers.registers8[prefixOpcode.column3]
                             : ReadBusRef(_registers.hl.reg);

    if (prefixOpcode.column3 == 06)
        _cyclesThisInstruction += 4;
    
    if (prefixOpcode.row5 == 00)
        return Rlc();
    if (prefixOpcode.row5 == 01)
        return Rrc();
    if (prefixOpcode.row5 == 02)
        return Rl();
    if (prefixOpcode.row5 == 03)
        return Rr();
    if (prefixOpcode.row5 == 04)
        return Sla();
    if (prefixOpcode.row5 == 05)
        return Sra();
    if (prefixOpcode.row5 == 06)
        return Swap();
    if (prefixOpcode.row5 == 07)
        return Srl();

    const byte testBit = (prefixOpcode.row5 - 010) % 010;
    if (prefixOpcode.row5 > 017 && prefixOpcode.row5 < 030)
        return Res(testBit, target);
    if (prefixOpcode.row5 > 027)
        return Set(testBit, target);

    LOG("Prefix function Op Code not found. Row " << static_cast<int>(prefixOpcode.row5) << ", column " << static_cast<
        int>(prefixOpcode.column3));
}

void Cpu::Ld8(byte& target, const byte source)
{
    target = source;
}

void Cpu::Ld16(word& target, const word source)
{
    target = source;
}

void Cpu::LdHlSpE8()
{
    _registers.f.z = 0;
    _registers.f.n = 0;

    const signed_byte e8 = static_cast<signed_byte>(ReadAtPcInc());
    const word newSp = _registerSp.reg + static_cast<word>(e8);
    _registers.hl.reg = newSp;

    _registers.f.h = (newSp & 0xf) + (e8 & 0xf);
    _registers.f.c = (newSp & 0xff) + (e8 & 0xff);

    _cyclesThisInstruction += 4;
}

void Cpu::Halt()
{
    LOG("Function HALT not implemented");
}

void Cpu::Add(const byte val)
{
    const byte prev = _registers.a;
    const word res = static_cast<word>(_registers.a) + static_cast<word>(val);

    _registers.a = static_cast<byte>(res);
    
    _registers.f.z = _registers.a == 0;
    _registers.f.n = 0;
    _registers.f.h = (prev & 0xF) + (val & 0xf) >= 0x10;
    _registers.f.c = res >= 0x100;
}

void Cpu::Adc(const byte val)
{
    const byte prev = _registers.a;
    const word res = static_cast<word>(_registers.a) + static_cast<word>(val) + static_cast<word>(_registers.f.c);

    _registers.a = static_cast<byte>(res);
    
    _registers.f.z = _registers.a == 0;
    _registers.f.n = 0;
    _registers.f.h = (prev & 0xF) + (val & 0xf) >= 0x10;
    _registers.f.c = res >= 0x100;
}

void Cpu::Sub(const byte val)
{
    const byte prev = _registers.a;
    const word res = static_cast<word>(_registers.a) - static_cast<word>(val);

    _registers.a = static_cast<byte>(res);
    
    _registers.f.z = _registers.a == 0;
    _registers.f.n = 1;
    _registers.f.h = (prev & 0xF) - (val & 0xf) < 0;
    _registers.f.c = static_cast<signed_word>(res) < 0;
}

void Cpu::Sbc(const byte val)
{
    const byte prev = _registers.a;
    const word res = static_cast<word>(_registers.a) - static_cast<word>(val) - static_cast<word>(_registers.f.c);

    _registers.a = static_cast<byte>(res);
    
    _registers.f.z = _registers.a == 0;
    _registers.f.n = 1;
    _registers.f.h = (prev & 0xF) - (val & 0xf) < 0;
    _registers.f.c = static_cast<signed_word>(res) < 0;
}

void Cpu::And(const byte val)
{
    _registers.a &= val;

    _registers.f.z = _registers.a == 0;
    _registers.f.n = 0;
    _registers.f.h = 1;
    _registers.f.c = 0;
}

void Cpu::Xor(const byte val)
{
    _registers.a ^= val;

    _registers.f.reg = 0;
    _registers.f.z = _registers.a == 0;
}

void Cpu::Or(const byte val)
{
    _registers.a |= val;

    _registers.f.z = _registers.a == 0;
    _registers.f.n = 0;
    _registers.f.h = 0;
    _registers.f.c = 0;
}

void Cpu::Cp(const byte val)
{
    const word res = static_cast<word>(_registers.a) - static_cast<word>(val);

    _registers.f.z = !(res & 0xFF);
    _registers.f.n = 1;
    _registers.f.h = (_registers.a & 0xF) - (val & 0xf) < 0;
    _registers.f.c = static_cast<signed_word>(res) < 0;
}

void Cpu::Nop()
{
    LOG("Function NOP not implemented");
}

void Cpu::Stop()
{
    LOG("Function STOP not implemented");
}

void Cpu::Jr(const byte test)
{
    const signed_byte e8 = static_cast<signed_byte>(ReadAtPcInc());
    const word newPc = _registerPc + static_cast<word>(e8);

    if (!test)
    {
        return;
    }

    _registerPc = newPc;
    _cyclesThisInstruction += 4;
}

void Cpu::Ret(const byte test)
{
    LOG("Function RET not implemented");
}

void Cpu::Pop()
{
    LOG("Function POP not implemented");
}

void Cpu::AddSp()
{
    LOG("Function AddSp not implemented");
}

void Cpu::Jp()
{
    LOG("Function JP not implemented");
}

void Cpu::Inc8(byte& target)
{
    const byte prev = target;
    target++;

    _registers.f.z = target == 0;
    _registers.f.n = 0;
    _registers.f.h = (prev & 0xF) == 0xF;
}

void Cpu::Inc16(word& target)
{
    target++;
}

void Cpu::Di()
{
    LOG("Function DI not implemented");
}

void Cpu::Ei()
{
    LOG("Function EI not implemented");
}

void Cpu::Call(const byte test, const word address)
{
    LOG("Function CALL not implemented");
}

void Cpu::Dec(byte& target)
{
    LOG("Function DEC not implemented");
}

void Cpu::Dec16(word& target)
{
    LOG("Function DEC16 not implemented");
}

void Cpu::Add16(word& target)
{
    LOG("Function ADD16 not implemented");
}

void Cpu::Push()
{
    LOG("Function PUSH not implemented");
}

void Cpu::Rlca()
{
    LOG("Function RLCA not implemented");
}

void Cpu::Rla()
{
    LOG("Function RLA not implemented");
}

void Cpu::Daa()
{
    LOG("Function DAA not implemented");
}

void Cpu::Scf()
{
    LOG("Function Scf not implemented");
}

void Cpu::Rst()
{
    LOG("Function Rsf not implemented");
}

void Cpu::Reti()
{
    LOG("Function Reti not implemented");
}

void Cpu::Rrca()
{
    LOG("Function Rrca not implemented");
}

void Cpu::Rra()
{
    LOG("Function Rra not implemented");
}

void Cpu::Cpl()
{
    LOG("Function Cpl not implemented");
}

void Cpu::Ccf()
{
    LOG("Function Ccf not implemented");
}

void Cpu::Rlc()
{
    LOG("Function Rlc not implemented");
}

void Cpu::Rrc()
{
    LOG("Function Rrc not implemented");
}

void Cpu::Rl()
{
    LOG("Function Rl not implemented");
}

void Cpu::Rr()
{
    LOG("Function Rr not implemented");
}

void Cpu::Sla()
{
    LOG("Function Sla not implemented");
}

void Cpu::Sra()
{
    LOG("Function Sra not implemented");
}

void Cpu::Swap()
{
    LOG("Function Swap not implemented");
}

void Cpu::Srl()
{
    LOG("Function Srl not implemented");
}

void Cpu::Bit(const byte testBit, const byte testR8)
{
    _registers.f.z = (1 << testBit & testR8) == 0;
    _registers.f.n = 0;
    _registers.f.h = 1;
}

void Cpu::Res(const byte testBit, const byte testR8)
{
    LOG("Function Res not implemented");
}

void Cpu::Set(const byte testBit, const byte testR8)
{
    LOG("Function Set not implemented");
}
