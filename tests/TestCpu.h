#pragma once

#include "Emulator/Cpu.h"
#include "Core/Definitions.h"

class Bus;

class TestCpu : public Cpu
{
public:
    explicit TestCpu(Bus* bus);

    // Public accessors for testing
    void SetA(byte value) { Cpu::SetA(value); }
    void SetF(byte value) { Cpu::SetF(value); }
    void SetB(byte value) { Cpu::SetB(value); }
    void SetC(byte value) { Cpu::SetC(value); }
    void SetD(byte value) { Cpu::SetD(value); }
    void SetE(byte value) { Cpu::SetE(value); }
    void SetH(byte value) { Cpu::SetH(value); }
    void SetL(byte value) { Cpu::SetL(value); }
    void SetSP(word value) { Cpu::SetSP(value); }
    void SetPC(word value) { Cpu::SetPC(value); }
    void SetIME(byte value) { Cpu::SetIME(value); }
    
    byte GetA() const { return Cpu::GetA(); }
    byte GetF() const { return Cpu::GetF(); }
    byte GetB() const { return Cpu::GetB(); }
    byte GetC() const { return Cpu::GetC(); }
    byte GetD() const { return Cpu::GetD(); }
    byte GetE() const { return Cpu::GetE(); }
    byte GetH() const { return Cpu::GetH(); }
    byte GetL() const { return Cpu::GetL(); }
    word GetSP() const { return Cpu::GetSP(); }
    word GetPC() const { return Cpu::GetPC(); }
    byte GetIME() const { return Cpu::GetIME(); }
    
    void SetBC(word value) { Cpu::SetBC(value); }
    void SetDE(word value) { Cpu::SetDE(value); }
    void SetHL(word value) { Cpu::SetHL(value); }
    word GetBC() const { return Cpu::GetBC(); }
    word GetDE() const { return Cpu::GetDE(); }
    word GetHL() const { return Cpu::GetHL(); }
    
    // Flag getters for testing
    bool GetFlagZ() const { return Cpu::GetFlagZ(); }
    bool GetFlagN() const { return Cpu::GetFlagN(); }
    bool GetFlagH() const { return Cpu::GetFlagH(); }
    bool GetFlagC() const { return Cpu::GetFlagC(); }
    
    // Convenience method to initialize to post-boot ROM state
    void InitializePostBootState();
};