#include "TestCpu.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/AddressConstants.h"

TestCpu::TestCpu(Bus* bus) : Cpu(bus)
{
}

void TestCpu::InitializePostBootState()
{
    // Initialize CPU registers to post-boot ROM state
    // Based on https://gbdev.io/pandocs/Power_Up_Sequence.html
    // Values for DMG (original Game Boy)
    
    // 8-bit registers
    SetA(0x01);  // A = $01
    SetF(0x80);  // F = $80 (Z=1, N=0, H=0, C=0 - assuming header checksum is $00)
    SetB(0x00);  // B = $00
    SetC(0x13);  // C = $13
    SetD(0x00);  // D = $00
    SetE(0xD8);  // E = $D8
    SetH(0x01);  // H = $01
    SetL(0x4D);  // L = $4D (HL = $014D)
    
    // 16-bit registers
    SetSP(0xFFFE);  // SP = $FFFE
    SetPC(0x0100);  // PC = $0100 (start of cartridge code)
    
    // Interrupt Master Enable
    SetIME(0x00);   // Interrupts disabled initially
}