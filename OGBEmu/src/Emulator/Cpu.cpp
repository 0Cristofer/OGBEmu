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

Cpu::Cpu(Bus* bus, const unsigned int framesPerSecond) : _bus(bus), _registerAF(), _registerBC(), _registerDE(),
                                                         _registerHL(), _registerSp(),
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
    _registerPc.reg = 0;
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
    const byte byteOpcode = _bus->Read(_registerPc.reg);
    _registerPc.reg++;

    Opcode opcode;
    opcode.code = byteOpcode;

    return opcode;
}

byte Cpu::ExecuteOpcode(const Opcode opcode)
{
    if (opcode.row > 0x3 && opcode.row < 0xC)
    {
        return ExecuteRowFunction(opcode);
    }

    return ExecuteColumnFunction(opcode);
}

byte Cpu::ExecuteRowFunction(const Opcode opcode)
{
    if (opcode.row > 0x3 && opcode.row < 0x8)
    {
        if (opcode.row == 0x7 && opcode.column == 0x6)
        {
            return Halt();
        }

        return Ld();
    }

    if (opcode.row == 0x8)
    {
        if (opcode.column < 0x8)
        {
            return Add();
        }

        return Adc();
    }

    if (opcode.row == 0x9)
    {
        if (opcode.column < 0x8)
        {
            return Sub();
        }

        return Sbc();
    }

    if (opcode.row == 0xA)
    {
        if (opcode.column < 0x8)
        {
            return And();
        }

        return Xor();
    }

    if (opcode.row == 0xB)
    {
        if (opcode.column < 0x8)
        {
            return Or();
        }

        return Cp();
    }

    LOG("Row function Op Code not found. Row " << static_cast<int>(opcode.row) << ", column " << static_cast<int>(opcode.column));
    return 4;
}

byte Cpu::ExecuteColumnFunction(const Opcode opcode)
{
    if (opcode.column == 0x0)
    {
        if (opcode.row == 0x0)
        {
            return Nop();
        }

        if (opcode.row == 0x1)
        {
            return Stop();
        }

        if (opcode.row == 0x2 || opcode.row == 0x3)
        {
            return Jr();
        }

        if (opcode.row == 0xC || opcode.row == 0xD)
        {
            return Ret();
        }

        if (opcode.row == 0xE || opcode.row == 0xF)
        {
            return Ld();
        }
    }

    if (opcode.column == 0x1)
    {
        if (opcode.row < 0x4)
        {
            return Ld();
        }

        if (opcode.row > 0xB)
        {
            return Pop();
        }
    }

    if (opcode.column == 0x2)
    {
        if (opcode.row < 0x4 || opcode.row > 0xD)
        {
            return Ld();
        }

        if (opcode.row == 0xC || opcode.row == 0xD)
        {
            return Jp();
        }
    }

    if (opcode.column == 0x3)
    {
        if (opcode.row < 0x4)
        {
            return Inc();
        }

        if (opcode.row == 0xC)
        {
            return Jp();
        }

        if (opcode.row == 0xF)
        {
            return Di();
        }
    }

    if (opcode.column == 0x4)
    {
        if (opcode.row < 0x4 || opcode.row > 0xD)
        {
            return Inc();
        }

        if (opcode.row == 0xC || opcode.row == 0xD)
        {
            return Call();
        }
    }

    if (opcode.column == 0x5)
    {
        if (opcode.row < 0x4)
        {
            return Dec();
        }

        if (opcode.row > 0xB)
        {
            return Push();
        }
    }

    if (opcode.column == 0x6)
    {
        if (opcode.row < 0x4)
        {
            return Ld();
        }

        if (opcode.row == 0xC)
        {
            return Add();
        }

        if (opcode.row == 0xD)
        {
            return Sub();
        }

        if (opcode.row == 0xE)
        {
            return And();
        }
        
        if (opcode.row == 0xF)
        {
            return Or();
        }
    }

    if (opcode.column == 0x7)
    {
        if (opcode.row == 0x0)
        {
            return Rlca();
        }
        
        if (opcode.row == 0x1)
        {
            return Rla();
        }

        if (opcode.row == 0x2)
        {
            return Daa();
        }

        if (opcode.row == 0x3)
        {
            return Scf();
        }

        if (opcode.row > 0xB)
        {
            return Rst();
        }
    }

    if (opcode.column == 0x8)
    {
        if (opcode.row == 0x0)
        {
            return Ld();
        }

        if (opcode.row > 0x0 && opcode.row < 0x4)
        {
            return Jr();
        }

        if (opcode.row == 0xC || opcode.row == 0xD)
        {
            return Ret();
        }

        if (opcode.row == 0xE)
        {
            return Add();
        }

        if (opcode.row == 0xF)
        {
            return Ld();
        }
    }

    if (opcode.column == 0x9)
    {
        if (opcode.row < 0x4)
        {
            return Add();
        }

        if (opcode.row == 0xC)
        {
            return Ret();
        }

        if (opcode.row == 0xD)
        {
            return Reti();
        }

        if (opcode.row == 0xE)
        {
            return Jp();
        }
        
        if (opcode.row == 0xF)
        {
            return Ld();
        }
    }

    if (opcode.column == 0xA)
    {
        if (opcode.row < 0x4)
        {
            return Ld();
        }

        if (opcode.row == 0xC || opcode.row == 0xD)
        {
            return Jp();
        }

        if (opcode.row == 0xE || opcode.row == 0xF)
        {
            return Ld();
        }
    }

    if (opcode.column == 0xB)
    {
        if (opcode.row < 0x4)
        {
            return Dec();
        }

        if (opcode.row == 0xC)
        {
            return Prefix();
        }
    }

    if (opcode.column == 0xC)
    {
        if (opcode.row < 0x4)
        {
            return Inc();
        }

        if (opcode.row == 0xC || opcode.row == 0xD)
        {
            return Call();
        }
    }

    if (opcode.column == 0xD)
    {
        if (opcode.row < 0x4)
        {
            return Dec();
        }

        if (opcode.row == 0xC)
        {
            return Call();
        }
    }

    if (opcode.column == 0xE)
    {
        if (opcode.row < 0x4)
        {
            return Ld();
        }

        if (opcode.row == 0xC)
        {
            return Adc();
        }

        if (opcode.row == 0xD)
        {
            return Sbc();
        }

        if (opcode.row == 0xE)
        {
            return Xor();
        }
        
        if (opcode.row == 0xF)
        {
            return Cp();
        }
    }

    if (opcode.column == 0xF)
    {
        if (opcode.row == 0x0)
        {
            return Rrca();
        }

        if (opcode.row == 0x1)
        {
            return Rra();
        }

        if (opcode.row == 0x2)
        {
            return Cpl();
        }

        if (opcode.row == 0x3)
        {
            return Ccf();
        }

        if (opcode.row > 0xB)
        {
            return Rst();
        }
    }
    
    LOG("Column function Op Code not found. Row " << static_cast<int>(opcode.row) << ", column " << static_cast<int>(opcode.column));
    return 4;
}

byte Cpu::Ld()
{
    LOG("Function LD not implemented");
    return 4;
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

byte Cpu::Xor()
{
    LOG("Function XOR not implemented");
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

void Cpu::SetFlag(const FlagBit flagBit, const byte val)
{
    _registerAF.lo |= val << flagBit;
}

byte Cpu::GetFlag(const FlagBit flagBit) const
{
    return _registerAF.lo & 1 << flagBit;
}
