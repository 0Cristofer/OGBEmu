#include "Cpu.h"

#include "Core/Logger.h"

#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/Bus.h"

Cpu::Cpu(Bus* bus) : _registers(), _registerSp(), _bus(bus), _eiRequested(false)
{
    // This is the only hardware initialization needed, everything else is done by the boot rom
    _ime = 0;
    _halted = 0;
    _registerPc.reg = 0;
    _bus->Write(AddressConstants::BootRomBank, 0);
    
    _bus->Write(0xff44, 0x90); // Hack to force boot with no screen
}

byte Cpu::Update()
{
    _cyclesThisInstruction = 0;

    if (!_halted)
    {
        const Opcode opcode = FetchNextOpcode();
        ExecuteOpcode(opcode);   
    }
    else
        _cyclesThisInstruction += 4;

    UpdateIme();
    HandleInterrupts();

    return _cyclesThisInstruction;
}

Opcode Cpu::FetchNextOpcode()
{
    Opcode opcode;
    opcode.code = ReadAtPcInc();

    return opcode;
}

void Cpu::UpdateIme()
{
    // Since EI needs to wait one instruction to be effective, we check if the previous instruction WASN't an EI (which means we're in the next instruction) to finalize it.
    if (_eiRequested && _bus->Read(_registerPc.reg - 1) != 0xFB)
    {
        _eiRequested = false;
        _ime = 1;
    }
}

void Cpu::HandleInterrupts()
{
    const byte interruptEnable = _bus->Read(AddressConstants::StartIeAddress);
    byte& interruptFlag = _bus->ReadRef(AddressConstants::InterruptFlag);
    const byte interruptsOccurred = interruptEnable & interruptFlag;

    if (!interruptsOccurred)
        return;

    _halted = 0;
    
    if (!_ime)
        return;

    _ime = 0;

    const byte vBlank = interruptsOccurred & 0b00000001;
    const byte lcd = interruptsOccurred & 0b00000010;
    const byte timer = interruptsOccurred & 0b00000100;
    const byte serial = interruptsOccurred & 0b00001000;
    const byte joypad = interruptsOccurred & 0b00010000;
    byte handledInterrupt;

    _cyclesThisInstruction += 8;
    Push(_registerPc);

    if (vBlank)
    {
        _registerPc.reg = AddressConstants::VBlankHandlerAddress;
        handledInterrupt = vBlank;
    }
    else if (lcd)
    {
        _registerPc.reg = AddressConstants::LcdHandlerAddress;
        handledInterrupt = lcd;
    }
    else if (timer)
    {
        _registerPc.reg = AddressConstants::TimerHandlerAddress;
        handledInterrupt = timer;
    }
    else if (serial)
    {
        _registerPc.reg = AddressConstants::SerialHandlerAddress;
        handledInterrupt = serial;
    }
    else if (joypad)
    {
        _registerPc.reg = AddressConstants::JoypadHandlerAddress;
        handledInterrupt = joypad;
    }
    else
    {
        LOG("Unknown interrupt: IE: " << interruptEnable << ", IF: " << interruptFlag);
        _registerPc.reg = AddressConstants::JoypadHandlerAddress;
        handledInterrupt = 0b00010000;
    }

    interruptFlag ^= handledInterrupt; // "Acknowledge" the interrupt by zeroing its bit
    
    _cyclesThisInstruction += 4;
}

byte Cpu::ReadAtPcInc()
{
    return ReadBus(_registerPc.reg++);
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

byte Cpu::ReadAtSp() const
{
    return _bus->Read(_registerSp.reg);
}

void Cpu::WriteAtSp(const byte data) const
{
    _bus->Write(_registerSp.reg, data);
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
            return Ld8(ReadBusRef(_registers.hl.reg), _registers.registers8[ConvertReg8Index(sourceIndex)]);
        if (sourceIndex == 6)
            return Ld8(_registers.registers8[ConvertReg8Index(targetIndex)], ReadBusRef(_registers.hl.reg));
        return Ld8(_registers.registers8[ConvertReg8Index(targetIndex)], _registers.registers8[ConvertReg8Index(sourceIndex)]);
    }

    if (opcode.row5 == 020)
    {
        if (opcode.column3 == 06)
            return Add(ReadBus(_registers.hl.reg));
        return Add(_registers.registers8[ConvertReg8Index(opcode.column3)]);
    }

    if (opcode.row5 == 021)
    {
        if (opcode.column3 == 06)
            return Adc(ReadBus(_registers.hl.reg));
        return Adc(_registers.registers8[ConvertReg8Index(opcode.column3)]);
    }

    if (opcode.row5 == 022)
    {
        if (opcode.column3 == 06)
            return Sub(ReadBus(_registers.hl.reg));
        return Sub(_registers.registers8[ConvertReg8Index(opcode.column3)]);
    }

    if (opcode.row5 == 023)
    {
        if (opcode.column3 == 06)
            return Sbc(ReadBus(_registers.hl.reg));
        return Sbc(_registers.registers8[ConvertReg8Index(opcode.column3)]);
    }

    if (opcode.row5 == 024)
    {
        if (opcode.column3 == 06)
            return And(ReadBus(_registers.hl.reg));
        return And(_registers.registers8[ConvertReg8Index(opcode.column3)]);
    }

    if (opcode.row5 == 025)
    {
        if (opcode.column3 == 06)
            return Xor(ReadBus(_registers.hl.reg));
        return Xor(_registers.registers8[ConvertReg8Index(opcode.column3)]);
    }

    if (opcode.row5 == 026)
    {
        if (opcode.column3 == 06)
            return Or(ReadBus(_registers.hl.reg));
        return Or(_registers.registers8[ConvertReg8Index(opcode.column3)]);
    }

    if (opcode.row5 == 027)
    {
        if (opcode.column3 == 06)
            return Cp(ReadBus(_registers.hl.reg));
        return Cp(_registers.registers8[ConvertReg8Index(opcode.column3)]);
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

            _cyclesThisInstruction += 4;
            return Ret(test);
        }
    }
    
    if (opcode.low == 0x0)
    {
        if (opcode.high == 0x0)
            return Nop();
        if (opcode.high == 0x1)
            return Stop();

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
            return Pop(_registers.registers16[opcode.high - 0xC]);
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
        {
            const byte flag = opcode.row5 < 032 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0x2 ? !flag : flag;
            
            return Jp(test, ReadImm16AtPc());
        }

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
            return Jp(1, ReadImm16AtPc());
        if (opcode.high == 0xF)
            return Di();
    }

    if (opcode.column3 == 04)
    {
        if (opcode.row5 == 06)
        {
            _cyclesThisInstruction += 4;
            return Inc8(ReadBusRef(_registers.hl.reg));
        }
        if (opcode.row5 < 010)
            return Inc8(_registers.registers8[ConvertReg8Index(opcode.row5)]);

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
            return Dec(_registers.registers8[ConvertReg8Index(opcode.row5)]);
    }

    if (opcode.low == 0x5)
    {
        if (opcode.high > 0xB && opcode.high < 0xf)
        {
            _cyclesThisInstruction += 4;
            return Push(_registers.registers16[opcode.high - 0xC]);
        }
        if (opcode.high == 0xf)
        {
            Register16 register16;
            register16.reg = static_cast<word>(static_cast<word>(_registers.a) << 8 | static_cast<int>(_registers.f.reg));

            _cyclesThisInstruction += 4;
            return Push(register16);
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
            return Ld8(_registers.registers8[ConvertReg8Index(targetRegister)], source);
        }
    }

    if (opcode.column3 == 07)
    {
        if (opcode.row5 > 027)
            return Rst((opcode.row5 - 030) * 0x8);
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
            return Rlca();
        if (opcode.high == 0x1)
            return Rla();
        if (opcode.high == 0x2)
            return Daa();
        if (opcode.high == 0x3)
            return Scf();
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
            return JpHl();

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
        if (opcode.high == 0xC)
            return Call(1, ReadImm16AtPc());

    if (opcode.low == 0xE)
    {
        if (opcode.high == 0xC)
            return Adc(ReadAtPcInc());
        if (opcode.high == 0xD)
            return Sbc(ReadAtPcInc());
        if (opcode.high == 0xE)
            return Xor(ReadAtPcInc());
        if (opcode.high == 0xF)
            return Cp(ReadAtPcInc());
    }

    if (opcode.low == 0xF)
    {
        if (opcode.high == 0x0)
            return Rrca();
        if (opcode.high == 0x1)
            return Rra();
        if (opcode.high == 0x2)
            return Cpl();
        if (opcode.high == 0x3)
            return Ccf();
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
                             ? _registers.registers8[ConvertReg8Index(prefixOpcode.column3)]
                             : ReadBus(_registers.hl.reg);
        const byte testBit = (prefixOpcode.row5 - 010) % 010;

        Bit(testBit, testR8);
        return;
    }
    
    byte& target = prefixOpcode.column3 != 06
                             ? _registers.registers8[ConvertReg8Index(prefixOpcode.column3)]
                             : ReadBusRef(_registers.hl.reg);

    if (prefixOpcode.column3 == 06)
        _cyclesThisInstruction += 4;
    
    if (prefixOpcode.row5 == 00)
        return Rlc(target);
    if (prefixOpcode.row5 == 01)
        return Rrc(target);
    if (prefixOpcode.row5 == 02)
        return Rl(target);
    if (prefixOpcode.row5 == 03)
        return Rr(target);
    if (prefixOpcode.row5 == 04)
        return Sla(target);
    if (prefixOpcode.row5 == 05)
        return Sra(target);
    if (prefixOpcode.row5 == 06)
        return Swap(target);
    if (prefixOpcode.row5 == 07)
        return Srl(target);

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
    _halted = 1;
}

void Cpu::Add(const byte val)
{
    const int diff = _registers.a + val;

    _registers.f.n = 0;
    _registers.f.h = (_registers.a & 0xF) + (val & 0xf) >= 0x10;
    _registers.f.c = diff >= 0x100;

    _registers.a = static_cast<byte>(diff);
    
    _registers.f.z = !_registers.a;
}

void Cpu::Adc(const byte val)
{
    const int diff = _registers.a + val + _registers.f.c;

    _registers.f.n = 0;
    _registers.f.h = (_registers.a & 0xF) + (val & 0xf) + _registers.f.c >= 0x10;
    _registers.f.c = diff >= 0x100;

    _registers.a = static_cast<byte>(diff);
    
    _registers.f.z = !_registers.a;
}

void Cpu::Sub(const byte val)
{
    const int diff = _registers.a - val;

    _registers.f.n = 1;
    _registers.f.h = (_registers.a & 0xF) - (val & 0xf) < 0;
    _registers.f.c = diff < 0;

    _registers.a = static_cast<byte>(diff);
    
    _registers.f.z = !_registers.a;
}

void Cpu::Sbc(const byte val)
{
    const int diff = _registers.a - val - _registers.f.c;

    _registers.f.n = 1;
    _registers.f.h = (_registers.a & 0xF) - (val & 0xf) - _registers.f.c < 0;
    _registers.f.c = diff < 0;

    _registers.a = static_cast<byte>(diff);
    
    _registers.f.z = !_registers.a;
}

void Cpu::And(const byte val)
{
    _registers.a &= val;

    _registers.f.z = !_registers.a;
    _registers.f.n = 0;
    _registers.f.h = 1;
    _registers.f.c = 0;
}

void Cpu::Xor(const byte val)
{
    _registers.a ^= val;

    _registers.f.reg = 0;
    _registers.f.z = !_registers.a;
}

void Cpu::Or(const byte val)
{
    _registers.a |= val;

    _registers.f.z = !_registers.a;
    _registers.f.n = 0;
    _registers.f.h = 0;
    _registers.f.c = 0;
}

void Cpu::Cp(const byte val)
{
    const int diff = _registers.a - val;

    _registers.f.n = 1;
    _registers.f.z = !(diff & 0xff);
    _registers.f.h = (_registers.a & 0xF) - (val & 0xf) < 0;
    _registers.f.c = diff < 0;
}

void Cpu::Nop()
{
}

void Cpu::Stop()
{
    LOG("Executing STOP");
    _registerPc.reg++;
}

void Cpu::Jr(const byte test)
{
    const signed_byte e8 = static_cast<signed_byte>(ReadAtPcInc());
    const word newPc = _registerPc.reg + static_cast<word>(e8);

    if (!test)
    {
        return;
    }

    _registerPc.reg = newPc;
    _cyclesThisInstruction += 4;
}

void Cpu::Ret(const byte test)
{
    if (!test)
        return;

    _cyclesThisInstruction += 4;

    Pop(_registerPc);
}

void Cpu::AddSp()
{
    const signed_byte e8 = static_cast<signed_byte>(ReadAtPcInc());

    _registers.f.z = 0;
    _registers.f.n = 0;
    _registers.f.c = (e8 & 0xff) + (_registerSp.reg & 0xFF) >= 0x100;
    _registers.f.h = (e8 & 0xf) + (_registerSp.reg & 0xF) >= 0x10;
    _registerSp.reg += e8;

    _cyclesThisInstruction += 8;
}

void Cpu::Jp(const byte test, const word address)
{
    if (!test)
        return;

    _registerPc.reg = address;
    _cyclesThisInstruction += 4;
}

void Cpu::JpHl()
{
    _registerPc.reg = _registers.hl.reg;
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
    _cyclesThisInstruction += 4;
    target++;
}

void Cpu::Di()
{
    _ime = 0;
}

void Cpu::Ei()
{
    _eiRequested = true;
}

void Cpu::Call(const byte test, const word address)
{
    if (!test)
        return;

    Push(_registerPc);
    Jp(1, address);
}

void Cpu::Dec(byte& target)
{
    const int diff = target - 1;
    
    _registers.f.h = (target & 0xF) == 0x0;
    target = static_cast<byte>(diff);
    _registers.f.n = 1;
    _registers.f.z = !target;
}

void Cpu::Dec16(word& target)
{
    _cyclesThisInstruction += 4;
    target--;
}

void Cpu::Add16(word& target)
{
    _cyclesThisInstruction += 4;
    target++;
}

void Cpu::Push(const Register16 register16Data)
{
    Dec16(_registerSp.reg);
    WriteAtSp(register16Data.hi);
    Dec16(_registerSp.reg);
    WriteAtSp(register16Data.lo);
}

void Cpu::Pop(Register16& register16Target)
{
    register16Target.lo = ReadAtSp();
    Inc16(_registerSp.reg);
    register16Target.hi = ReadAtSp();
    Inc16(_registerSp.reg);
}

void Cpu::Rlca()
{
    Rlc(_registers.a);
    _registers.f.z = 0;
}

void Cpu::Rla()
{
    Rl(_registers.a);
    _registers.f.z = 0;
}

void Cpu::Daa()
{
    LOG("Function DAA not implemented");
}

void Cpu::Scf()
{
    _registers.f.c = 1;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Rst(const word address)
{
    Call(1, address);
}

void Cpu::Reti()
{
    _ime = 1;
    Ret(1);
}

void Cpu::Rrca()
{
    Rrc(_registers.a);
    _registers.f.z = 0;
}

void Cpu::Rra()
{
    Rr(_registers.a);
    _registers.f.z = 0;
}

void Cpu::Cpl()
{
    _registers.a ^= 0xFF;
    _registers.f.h = 1;
    _registers.f.n = 1;
}

void Cpu::Ccf()
{
    _registers.f.c ^= 1;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Rlc(byte& reg)
{
    reg = static_cast<byte>(reg << 1 | reg >> 7);

    _registers.f.c = reg & 1;
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Rrc(byte& reg)
{
    const int low = reg & 1;
    reg = static_cast<byte>(reg >> 1 | low << 7);

    _registers.f.c = static_cast<byte>(low);
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Rl(byte& reg)
{
    const int wide = reg << 1 | _registers.f.c;
    reg = static_cast<byte>(wide);

    _registers.f.c = static_cast<byte>(wide >> 8);
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Rr(byte& reg)
{
    const int low = reg & 1;
    reg = static_cast<byte>(reg >> 1 | _registers.f.c << 7);

    _registers.f.c = static_cast<byte>(low);
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Sla(byte& reg)
{
    reg <<= 1;
    
    _registers.f.c = reg >> 7;
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Sra(byte& reg)
{
    reg = static_cast<byte>(static_cast<signed_byte>(reg) >> 1);
    
    _registers.f.c = reg & 1;
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Swap(byte& reg)
{
    reg = static_cast<byte>(reg << 4 | reg >> 4);

    _registers.f.c = 0;
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Srl(byte& reg)
{
    reg >>= 1;

    _registers.f.c = reg & 1;
    _registers.f.z = !reg;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Bit(const byte testBit, const byte testR8)
{
    _registers.f.z = !(1 << testBit & testR8);
    _registers.f.n = 0;
    _registers.f.h = 1;
}

void Cpu::Res(const byte testBit, byte& testR8)
{
    testR8 &= ~(1 << testBit);
}

void Cpu::Set(const byte testBit, byte& testR8)
{
    testR8 |= 1 << testBit;
}
