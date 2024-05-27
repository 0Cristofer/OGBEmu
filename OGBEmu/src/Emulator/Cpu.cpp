#include "Cpu.h"

#include <format>

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

    if (_registerPc.reg == 0x100)
    {
        DEBUGBREAKLOG("Finished boot");
        // Hack to run tests with no screen
        _bus->Write(0xff44, 0xff);
        _bus->Write(0xff02, 0xff);
    }

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
    const byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
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
    word jumpAddress;

    if (vBlank)
    {
        jumpAddress = AddressConstants::VBlankHandlerAddress;
        handledInterrupt = vBlank;
    }
    else if (lcd)
    {
        jumpAddress = AddressConstants::LcdHandlerAddress;
        handledInterrupt = lcd;
    }
    else if (timer)
    {
        jumpAddress = AddressConstants::TimerHandlerAddress;
        handledInterrupt = timer;
    }
    else if (serial)
    {
        jumpAddress = AddressConstants::SerialHandlerAddress;
        handledInterrupt = serial;
    }
    else if (joypad)
    {
        jumpAddress = AddressConstants::JoypadHandlerAddress;
        handledInterrupt = joypad;
    }
    else
    {
        DEBUGBREAKLOG("Unknown interrupt: IE: " << std::format("{:x}", interruptEnable) << ", IF: " << std::format("{:x}", interruptFlag));
        jumpAddress = AddressConstants::JoypadHandlerAddress;
        handledInterrupt = 0b00010000;
    }

    _cyclesThisInstruction += 4;
    WriteBus(AddressConstants::InterruptFlag, interruptFlag ^ handledInterrupt); // "Acknowledge" the interrupt by zeroing its bit
    Call(jumpAddress);
}

byte Cpu::ReadAtPcInc()
{
    return ReadBus(_registerPc.reg++);
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

void Cpu::WriteBus(const word address, const byte data)
{
    _cyclesThisInstruction += 4;
    _bus->Write(address, data);
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
    if (opcode.row5 > 07 && opcode.row5 < 020)
    {
        if (opcode.row5 == 016 && opcode.column3 == 06)
            return Halt();

        const byte targetIndex = opcode.row5 - 010;
        const byte sourceIndex = opcode.column3;

        if (targetIndex == 6)
            return Ld8Ta(_registers.hl.reg, sourceIndex);
        if (sourceIndex == 6)
            return Ld8Sa(targetIndex, _registers.hl.reg);

        if (opcode.row5 == 010 && opcode.column3 == 0)
            DEBUGBREAKLOG("LD B B");
        return Ld8R(targetIndex, sourceIndex);
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
    
    DEBUGBREAKLOG("Row function Op Code not found. Opcode: " << std::format("{:x}", opcode.code));
}

void Cpu::ExecuteLowFunction(const Opcode opcode)
{
    if (opcode.column3 == 00)
    {
        if (opcode.row5 == 03)
            return Jr(static_cast<signed_byte>(ReadAtPcInc()));
        if (opcode.row5 > 03 && opcode.row5 < 010)
        {
            const byte flag = opcode.row5 < 06 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0 ? !flag : flag;
            
            return JrTest(test, static_cast<signed_byte>(ReadAtPcInc()));
        }

        if (opcode.row5 > 027 && opcode.row5 < 034)
        {
            const byte flag = opcode.row5 < 032 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0 ? !flag : flag;

            _cyclesThisInstruction += 4;
            return RetTest(test);
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
            const word address = 0xff00 + static_cast<word>(ReadAtPcInc());
            constexpr byte aRegIndex = 0x7;

            if (opcode.high == 0xE)
                return Ld8Ta(address, aRegIndex);
            return Ld8Sa(aRegIndex, address);
        }
    }

    if (opcode.low == 0x1)
    {
        if (opcode.high < 0x4)
        {
            if (opcode.high == 0x3)
                return LdSpTImm();
            return Ld16Imm(opcode.row);
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
            constexpr byte aRegIndex = 0x7;

            if (opcode.row5 % 2 == 0)
            {
                word address;
                if (opcode.row == 0x2)
                    address = _registers.hl.reg++;
                else if (opcode.row == 0x3)
                    address = _registers.hl.reg--;
                else
                    address = _registers.registers16[opcode.row].reg;
                return Ld8Ta(address, aRegIndex);
            }

            word address;
            if (opcode.row == 0x2)
                address = _registers.hl.reg++;
            else if (opcode.row == 0x3)
                address = _registers.hl.reg--;
            else
                address = _registers.registers16[opcode.row].reg;

            return Ld8Sa(aRegIndex, address);
        }

        if (opcode.row5 > 027 && opcode.row5 < 034)
        {
            const byte flag = opcode.row5 < 032 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0x2 ? !flag : flag;
            
            return JpTest(test, ReadImm16AtPc());
        }

        if (opcode.row5 > 033)
        {
            constexpr byte aRegIndex = 0x7;

            if (opcode.row5 % 2 == 0)
            {
                const word address = 0xff00 + static_cast<word>(_registers.c);
                if (opcode.high == 0xE)
                    return Ld8Ta(address, aRegIndex);
                if (opcode.high == 0xF)
                    return Ld8Sa(aRegIndex, address);
            }

            if (opcode.high == 0xE)
                return Ld8Ta(ReadImm16AtPc(), aRegIndex);
            if (opcode.high == 0xF)
                return Ld8Sa(aRegIndex, ReadImm16AtPc());
        }
    }

    if (opcode.low == 0x3)
    {
        if (opcode.high < 0x3)
            return Inc16(_registers.registers16[opcode.high].reg);
        if (opcode.high == 0x4)
            return Inc16(_registerSp.reg);
        if (opcode.high == 0xC)
            return Jp(ReadImm16AtPc());
        if (opcode.high == 0xF)
            return Di();
    }

    if (opcode.column3 == 04)
    {
        if (opcode.row5 == 06)
            return Inc8Add(_registers.hl.reg);
        if (opcode.row5 < 010)
            return Inc8(_registers.registers8[ConvertReg8Index(opcode.row5)]);

        if (opcode.row5 > 027 && opcode.row5 < 034)
        {
            const byte flag = opcode.row5 < 032 ? _registers.f.z : _registers.f.c;
            const byte test = opcode.column == 0 ? !flag : flag;
            
            return CallTest(test, ReadImm16AtPc());
        }
    }

    if (opcode.column3 == 05)
    {
        if (opcode.row5 == 06)
            return Dec8Add(_registers.hl.reg);
        if (opcode.row5 < 010)
            return Dec8(_registers.registers8[ConvertReg8Index(opcode.row5)]);
    }

    if (opcode.low == 0x5)
    {
        if (opcode.high > 0xB)
        {
            _cyclesThisInstruction += 4;
            return Push(_registers.registers16[opcode.high - 0xC]);
        }
    }

    if (opcode.column3 == 06)
    {
        if (opcode.row5 < 010)
        {
            const byte targetRegister = opcode.row5;

            if (targetRegister == 0x6)
                return Ld8TaImm(_registers.hl.reg);
            return Ld8Imm(targetRegister);
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
            return LdImmTaSp();

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
            return Ret();
        if (opcode.high == 0xD)
            return Reti();
        if (opcode.high == 0xE)
            return JpHl();
        if (opcode.high == 0xF)
            return LdSpS(_registers.hl.reg);
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
            return Call(ReadImm16AtPc());

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

    DEBUGBREAKLOG("Column function Op Code not found. Opcode: " << std::format("{:x}", opcode.code));
}

void Cpu::ExecutePrefix()
{
    const Opcode prefixOpcode = FetchNextOpcode();
    
    const byte testBit = (prefixOpcode.row5 - 010) % 010;
    byte target = prefixOpcode.column3 == 06 ? ReadBus(_registers.hl.reg) : _registers.registers8[ConvertReg8Index(prefixOpcode.column3)];

    if (prefixOpcode.row5 == 00)
        Rlc(target);
    else if (prefixOpcode.row5 == 01)
        Rrc(target);
    else if (prefixOpcode.row5 == 02)
        Rl(target);
    else if (prefixOpcode.row5 == 03)
        Rr(target);
    else if (prefixOpcode.row5 == 04)
        Sla(target);
    else if (prefixOpcode.row5 == 05)
        Sra(target);
    else if (prefixOpcode.row5 == 06)
        Swap(target);
    else if (prefixOpcode.row5 == 07)
        Srl(target);
    else if (prefixOpcode.row5 > 07 && prefixOpcode.row5 < 020)
    {
        Bit(testBit, target);
        return;
    }
    else if (prefixOpcode.row5 > 017 && prefixOpcode.row5 < 030)
        Res(testBit, target);
    else if (prefixOpcode.row5 > 027)
        Set(testBit, target);

    if (prefixOpcode.column3 == 06)
        return WriteBus(_registers.hl.reg, target);
    _registers.registers8[ConvertReg8Index(prefixOpcode.column3)] = target;
}

void Cpu::Ld8R(const byte targetIndex, const byte sourceIndex)
{
    _registers.registers8[ConvertReg8Index(targetIndex)] = _registers.registers8[ConvertReg8Index(sourceIndex)];
}

void Cpu::Ld8Imm(const byte targetIndex)
{
    _registers.registers8[ConvertReg8Index(targetIndex)] = ReadAtPcInc();
}

void Cpu::Ld8TaImm(const word address)
{
    WriteBus(address, ReadAtPcInc());
}

void Cpu::Ld8Sa(const byte targetIndex, const word sourceAddress)
{
    _registers.registers8[ConvertReg8Index(targetIndex)] = ReadBus(sourceAddress);
}

void Cpu::Ld8Ta(const word targetAddress, const byte sourceIndex)
{
    WriteBus(targetAddress, _registers.registers8[ConvertReg8Index(sourceIndex)]);
}

void Cpu::Ld16Imm(const byte targetIndex)
{
    _registers.registers16[targetIndex].reg = ReadImm16AtPc();
}

void Cpu::LdSpTImm()
{
    _registerSp.reg = ReadImm16AtPc();
}

void Cpu::LdSpS(const word val)
{
    _cyclesThisInstruction += 4;
    _registerSp.reg = val;
}

void Cpu::LdImmTaSp()
{
    const word imm = ReadImm16AtPc();

    WriteBus(imm, _registerSp.lo);
    WriteBus(imm + 1, _registerSp.hi);
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
    DEBUGBREAKLOG("Executing STOP");
    _registerPc.reg++;
}

void Cpu::Jr(const signed_byte offset)
{
    const word newPc = _registerPc.reg + static_cast<word>(offset);

    _registerPc.reg = newPc;
    _cyclesThisInstruction += 4;
}

void Cpu::JrTest(const byte test, const signed_byte offset)
{
    if (!test)
        return;
    Jr(offset);
}

void Cpu::Ret()
{
    _cyclesThisInstruction += 4;
    Pop(_registerPc);
}

void Cpu::RetTest(const byte test)
{
    if (!test)
        return;

    Ret();
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

void Cpu::Jp(const word address)
{
    _registerPc.reg = address;
    _cyclesThisInstruction += 4;
}

void Cpu::JpTest(const byte test, const word address)
{
    if (!test)
        return;

    Jp(address);
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

void Cpu::Inc8Add(const word address)
{
    byte target = ReadBus(address);
    Inc8(target);
    WriteBus(address, target);
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

void Cpu::Call(const word address)
{
    Push(_registerPc);
    Jp(address);
}

void Cpu::CallTest(const byte test, const word address)
{
    if (!test)
        return;

    Call(address);
}

void Cpu::Dec8(byte& target)
{
    const int diff = target - 1;
    
    _registers.f.h = (target & 0xF) == 0x0;
    target = static_cast<byte>(diff);
    _registers.f.n = 1;
    _registers.f.z = !target;
}

void Cpu::Dec8Add(word address)
{
    byte target = ReadBus(address);
    Dec8(target);
    WriteBus(address, target);
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
    if (_registers.f.n)
    {
        if (_registers.f.h)
        {
            _registers.a += 0xFA;
        }
        if (_registers.f.c)
        {
            _registers.a += 0xA0;
        }
    }
    else
    {
        int a = _registers.a;
        if ((_registers.a & 0xF) > 0x9 || _registers.f.h)
        {
            a += 0x6;
        }
        if ((a & 0x1F0) > 0x90 || _registers.f.c)
        {
            a += 0x60;
            _registers.f.c = 1;
        }
        else
        {
            _registers.f.c = 0;
        }
        _registers.a = static_cast<byte>(a);
    }

    _registers.f.h = 0;
    _registers.f.z = !_registers.a;
}

void Cpu::Scf()
{
    _registers.f.c = 1;
    _registers.f.h = 0;
    _registers.f.n = 0;
}

void Cpu::Rst(const word address)
{
    Call(address);
}

void Cpu::Reti()
{
    _ime = 1;
    Ret();
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
