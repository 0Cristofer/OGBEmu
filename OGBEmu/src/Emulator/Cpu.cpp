#include "Cpu.h"

#include <chrono>

#include "AddressConstants.h"
#include "Core/Logger.h"

#include "Bus.h"
#include "CpuConstants.h"

namespace
{
    constexpr unsigned char DefaultSimulationFramesPerSecond = 64;
}

Cpu::Cpu(Bus* bus, const unsigned int framesPerSecond) : _bus(bus), _registers(), _registerSp(),
                                                         _framesPerSecond(framesPerSecond)
{
    if (!IsPowerOfTwo(_framesPerSecond))
    {
        LOG("Simulation frames per second must be a power of two, got " << _framesPerSecond << ", defaulting to " <<
            DefaultSimulationFramesPerSecond);
        _framesPerSecond = DefaultSimulationFramesPerSecond;
    }

    _frameTimeSeconds = 1./_framesPerSecond;
    _maxCyclesPerFrame = CpuConstants::CpuClock * _frameTimeSeconds;

    // This is the only hardware initialization needed, everything else is done by the boot rom
    _registerPc = 0;
    _bus->Write(AddressConstants::BootRomBank, 0);
}

void Cpu::Run()
{
    constexpr double maxRunSeconds = 5.;
    
    unsigned int totalCycles = 0;
    unsigned int totalFrames = 0;
    
    double runSeconds = 0;
    
    const auto runStartTime = std::chrono::steady_clock::now();
    while(runSeconds < maxRunSeconds)
    {
        const unsigned int cyclesDone = DoFrame();

        if (cyclesDone == 0)
        {
            return;
        }

        const auto runCurrentTime = std::chrono::steady_clock::now();
        runSeconds = std::chrono::duration<double>(runCurrentTime - runStartTime).count();

        totalCycles += cyclesDone;
        totalFrames++;
    }

    LOG("Finished running. Ran for " << runSeconds << "s, with " << totalFrames << " frames and cycled " << totalCycles << " times");
}

unsigned int Cpu::DoFrame()
{
    unsigned int cycleCount = 0;
    
    const auto frameStartTime = std::chrono::steady_clock::now();
    while (cycleCount < _maxCyclesPerFrame)
    {
        const byte cyclesExecuted = ExecuteNextInstruction();
        if (cyclesExecuted == 0)
        {
            return cycleCount;
        }
        
        cycleCount += cyclesExecuted;
    }
    const auto frameEndTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double> frameTime = frameEndTime - frameStartTime;

    WaitForNextFrame(frameTime.count());

    return cycleCount;
}

void Cpu::WaitForNextFrame(const double frameTimeSeconds) const
{
    const double remainingFrameTime = _frameTimeSeconds - frameTimeSeconds;

    if (remainingFrameTime < 0)
    {
        LOG("Running behind, last frame took: " << frameTimeSeconds << "s");
    }

    double remainingWaitTime = remainingFrameTime;
    
    const auto waitingStartTime = std::chrono::steady_clock::now();
    while(remainingWaitTime > 0)
    {
        const auto waitingEndTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double> waitingTime = waitingEndTime - waitingStartTime;
        remainingWaitTime = remainingFrameTime - waitingTime.count();
    }
}

byte Cpu::ExecuteNextInstruction()
{
    const Opcode opcode = FetchNextOpcode();
    return ExecuteOpcode(opcode);
}

Opcode Cpu::FetchNextOpcode()
{
    Opcode opcode;
    opcode.code = ReadAtPcInc();

    return opcode;
}

byte Cpu::ReadAtPcInc()
{
    const byte byte = _bus->Read(_registerPc);
    _registerPc++;

    return byte;
}

byte Cpu::ExecuteOpcode(const Opcode opcode)
{
    if (opcode.high > 0x3 && opcode.high < 0xC)
    {
        return ExecuteHighFunction(opcode);
    }

    return ExecuteLowFunction(opcode);
}

byte Cpu::ExecuteHighFunction(const Opcode opcode)
{
    if (opcode.high > 0x3 && opcode.high < 0x8)
    {
        if (opcode.high == 0x7 && opcode.low == 0x6)
        {
            return Halt();
        }

        const byte targetIndex = opcode.row6 - 010;
        const byte sourceIndex = opcode.column2;
        
        if (targetIndex == 6)
        {
            return 4 + Ld8(_bus->ReadRef(_registers.hl), _registers.registers8[sourceIndex]);
        }

        if (sourceIndex == 6)
        {
            return 4 + Ld8(_registers.registers8[targetIndex], _bus->ReadRef(_registers.hl));
        }
        
        return Ld8(_registers.registers8[targetIndex], _registers.registers8[sourceIndex]);
    }

    if (opcode.high == 0x8)
    {
        if (opcode.low < 0x8)
        {
            return Add();
        }

        return Adc();
    }

    if (opcode.high == 0x9)
    {
        if (opcode.low < 0x8)
        {
            return Sub();
        }

        return Sbc();
    }

    if (opcode.high == 0xA)
    {
        if (opcode.low < 0x8)
        {
            return And();
        }

        if (opcode.low == 0xE)
        {
            return 4 + Xor(_bus->Read(_registers.hl));
        }
        
        const byte sourceIndex = opcode.column2;
        return Xor(_registers.registers8[sourceIndex]);
    }

    if (opcode.high == 0xB)
    {
        if (opcode.low < 0x8)
        {
            return Or();
        }

        return Cp();
    }

    LOG("Row function Op Code not found. Row " << static_cast<int>(opcode.high) << ", column " << static_cast<int>(opcode.low));
    return 4;
}

byte Cpu::ExecuteLowFunction(const Opcode opcode)
{
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

        if (opcode.high == 0x2 || opcode.high == 0x3)
        {
            return Jr();
        }

        if (opcode.high == 0xC || opcode.high == 0xD)
        {
            return Ret();
        }

        if (opcode.high == 0xE || opcode.high == 0xF)
        {
            const word imm = 0xff00 + static_cast<word>(ReadAtPcInc());
            byte& valAtImm = _bus->ReadRef(imm);

            if (opcode.high == 0xE)
            {
                return 8 + Ld8(valAtImm, _registers.a);
            }

            return 8 + Ld8(_registers.a, valAtImm);
        }
    }

    if (opcode.low == 0x1)
    {
        if (opcode.high < 0x4)
        {
            word& target = opcode.high == 0x3 ? _registerSp.reg : _registers.registers16[opcode.row];

            return 8 + Ld16(target, ReadImm16AtPc());
        }

        if (opcode.high > 0xB)
        {
            return Pop();
        }
    }

    if (opcode.low == 0x2)
    {
        if (opcode.high < 0x2)
        {
            return 4 + Ld8(_bus->ReadRef(_registers.registers16[opcode.row]), _registers.a);
        }
        
        if (opcode.high == 0x2)
        {
            byte& target = _bus->ReadRef(_registers.hl);
            _registers.hl++;

            return 4 + Ld8(target, _registers.a);
        }

        if (opcode.high == 0x3)
        {
            byte& target = _bus->ReadRef(_registers.hl);
            _registers.hl--;
            
            return 4 + Ld8(target, _registers.a);
        }

        if (opcode.high == 0xC || opcode.high == 0xD)
        {
            return Jp();
        }

        const word address = 0xff00 + static_cast<word>(_registers.c);

        if (opcode.high == 0xE)
        {
            return 4 + Ld8(_bus->ReadRef(address), _registers.a);
        }

        if (opcode.high == 0xF)
        {
            return 4 + Ld8(_registers.a, _bus->Read(address));
        }
    }

    if (opcode.low == 0x3)
    {
        if (opcode.high < 0x4)
        {
            return Inc();
        }

        if (opcode.high == 0xC)
        {
            return Jp();
        }

        if (opcode.high == 0xF)
        {
            return Di();
        }
    }

    if (opcode.low == 0x4)
    {
        if (opcode.high < 0x4 || opcode.high > 0xD)
        {
            return Inc();
        }

        if (opcode.high == 0xC || opcode.high == 0xD)
        {
            return Call();
        }
    }

    if (opcode.low == 0x5)
    {
        if (opcode.high < 0x4)
        {
            return Dec();
        }

        if (opcode.high > 0xB)
        {
            return Push();
        }
    }

    if (opcode.low == 0x6 || opcode.low == 0xE)
    {
        if (opcode.high < 0x4)
        {
            const byte source = ReadAtPcInc();
            const byte targetRegister = opcode.row6;

            if (targetRegister == 0x6)
            {
                return 8 + Ld8(_bus->ReadRef(_registers.hl), source);
            }
            
            return 4 + Ld8(_registers.registers8[targetRegister], source);
        }
    }

    if (opcode.low == 0x6)
    {
        if (opcode.high == 0xC)
        {
            return Add();
        }

        if (opcode.high == 0xD)
        {
            return Sub();
        }

        if (opcode.high == 0xE)
        {
            return And();
        }
        
        if (opcode.high == 0xF)
        {
            return Or();
        }
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

        if (opcode.high > 0xB)
        {
            return Rst();
        }
    }

    if (opcode.low == 0x8)
    {
        if (opcode.high == 0x0)
        {
            const word imm = ReadImm16AtPc();
            
            return 12 + Ld8(_bus->ReadRef(imm), _registerSp.hi) + Ld8(_bus->ReadRef(imm+1), _registerSp.lo);
        }

        if (opcode.high > 0x0 && opcode.high < 0x4)
        {
            return Jr();
        }

        if (opcode.high == 0xC || opcode.high == 0xD)
        {
            return Ret();
        }

        if (opcode.high == 0xE)
        {
            return Add();
        }

        if (opcode.high == 0xF)
        {
            return LdHlSpE8();
        }
    }

    if (opcode.low == 0x9)
    {
        if (opcode.high < 0x4)
        {
            return Add();
        }

        if (opcode.high == 0xC)
        {
            return Ret();
        }

        if (opcode.high == 0xD)
        {
            return Reti();
        }

        if (opcode.high == 0xE)
        {
            return Jp();
        }
        
        if (opcode.high == 0xF)
        {
            return 4 + Ld16(_registerSp.reg, _registers.hl);
        }
    }

    if (opcode.low == 0xA)
    {
        if (opcode.high < 0x2)
        {
            return 4 + Ld8(_registers.a, _bus->ReadRef(_registers.registers16[opcode.row]));
        }
        
        if (opcode.high == 0x2)
        {
            const byte source = _bus->Read(_registers.hl);
            _registers.hl++;

            return 4 + Ld8(_registers.a, source);
        }

        if (opcode.high == 0x3)
        {
            const byte source = _bus->Read(_registers.hl);
            _registers.hl--;
            
            return 4 + Ld8(_registers.a, source);
        }

        if (opcode.high == 0xC || opcode.high == 0xD)
        {
            return Jp();
        }

        if (opcode.high == 0xE)
        {
            return 12 + Ld8(_bus->ReadRef(ReadImm16AtPc()), _registers.a);
        }

        if (opcode.high == 0xF)
        {
            return 12 + Ld8(_registers.a, _bus->ReadRef(ReadImm16AtPc()));
        }
    }

    if (opcode.low == 0xB)
    {
        if (opcode.high < 0x4)
        {
            return Dec();
        }

        if (opcode.high == 0xC)
        {
            return 4 + Prefix();
        }
    }

    if (opcode.low == 0xC)
    {
        if (opcode.high < 0x4)
        {
            return Inc();
        }

        if (opcode.high == 0xC || opcode.high == 0xD)
        {
            return Call();
        }
    }

    if (opcode.low == 0xD)
    {
        if (opcode.high < 0x4)
        {
            return Dec();
        }

        if (opcode.high == 0xC)
        {
            return Call();
        }
    }

    if (opcode.low == 0xE)
    {
        if (opcode.high == 0xC)
        {
            return Adc();
        }

        if (opcode.high == 0xD)
        {
            return Sbc();
        }

        if (opcode.high == 0xE)
        {
            return 4 + Xor(ReadAtPcInc());
        }
        
        if (opcode.high == 0xF)
        {
            return Cp();
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

        if (opcode.high > 0xB)
        {
            return Rst();
        }
    }
    
    LOG("Column function Op Code not found. Row " << static_cast<int>(opcode.high) << ", column " << static_cast<int>(opcode.low));
    return 4;
}

word Cpu::ReadImm16AtPc()
{
    const byte firstByte = ReadAtPcInc();
    const word imm = static_cast<word>(firstByte) + static_cast<word>(ReadAtPcInc() << 8);

    return imm;
}

byte Cpu::Ld8(byte& target, const byte source)
{
    target = source;
    return 4;
}

byte Cpu::Ld16(word& target, word source)
{
    target = source;
    return 4;
}

byte Cpu::LdHlSpE8()
{
    _registers.f.z = 0;
    _registers.f.n = 0;
    
    const signed_byte e8 = static_cast<signed_byte>(ReadAtPcInc());
    const word newSp = _registerSp.reg + static_cast<word>(e8);
    _registers.hl = newSp;
    
    _registers.f.h = (newSp & 0xf) + (e8 & 0xf);
    _registers.f.c = (newSp & 0xff) + (e8 & 0xff);
    
    return 12;
}

byte Cpu::Halt()
{
    LOG("Function HALT not implemented");
    return 4;
}

byte Cpu::Add()
{
    LOG("Function ADD not implemented");
    return 4;
}

byte Cpu::Adc()
{
    LOG("Function ADC not implemented");
    return 4;
}

byte Cpu::Sub()
{
    LOG("Function SUB not implemented");
    return 4;
}

byte Cpu::Sbc()
{
    LOG("Function SBC not implemented");
    return 4;
}

byte Cpu::And()
{
    LOG("Function AND not implemented");
    return 4;
}

byte Cpu::Xor(const byte val)
{
    _registers.a ^= val;

    _registers.f.reg = 0;
    _registers.f.z = _registers.a == 0;
    
    return 4;
}

byte Cpu::Or()
{
    LOG("Function OR not implemented");
    return 4;
}

byte Cpu::Cp()
{
    LOG("Function CP not implemented");
    return 4;
}

byte Cpu::Nop()
{
    LOG("Function NOP not implemented");
    return 4;
}

byte Cpu::Stop()
{
    LOG("Function STOP not implemented");
    return 4;
}

byte Cpu::Jr()
{
    LOG("Function JR not implemented");
    return 4;
}

byte Cpu::Ret()
{
    LOG("Function RET not implemented");
    return 4;
}

byte Cpu::Pop()
{
    LOG("Function POP not implemented");
    return 4;
}

byte Cpu::Jp()
{
    LOG("Function JP not implemented");
    return 4;
}

byte Cpu::Inc()
{
    LOG("Function INC not implemented");
    return 4;
}

byte Cpu::Di()
{
    LOG("Function DI not implemented");
    return 4;
}

byte Cpu::Call()
{
    LOG("Function CALL not implemented");
    return 4;
}

byte Cpu::Dec()
{
    LOG("Function DEC not implemented");
    return 4;
}

byte Cpu::Push()
{
    LOG("Function PUSH not implemented");
    return 4;
}

byte Cpu::Rlca()
{
    LOG("Function RLCA not implemented");
    return 4;
}

byte Cpu::Rla()
{
    LOG("Function RLA not implemented");
    return 4;
}

byte Cpu::Daa()
{
    LOG("Function DAA not implemented");
    return 4;
}

byte Cpu::Scf()
{
    LOG("Function Scf not implemented");
    return 4;
}

byte Cpu::Rst()
{
    LOG("Function Rsf not implemented");
    return 4;
}

byte Cpu::Reti()
{
    LOG("Function Reti not implemented");
    return 4;
}

byte Cpu::Prefix()
{
    LOG("Function Prefix not implemented");
    return 4;
}

byte Cpu::Rrca()
{
    LOG("Function Rrca not implemented");
    return 4;
}

byte Cpu::Rra()
{
    LOG("Function Rra not implemented");
    return 4;
}

byte Cpu::Cpl()
{
    LOG("Function Cpl not implemented");
    return 4;
}

byte Cpu::Ccf()
{
    LOG("Function Ccf not implemented");
    return 4;
}
