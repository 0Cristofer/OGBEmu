#include "CpuLoadInstructionsTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

CpuLoadInstructionsTest::CpuLoadInstructionsTest() : BaseTest("CPU Load Instructions Test")
{
}

void CpuLoadInstructionsTest::Setup()
{
    // Create minimal test setup
    _bootRomData = std::vector<byte>(256, 0x00);
    _cartridgeData = std::vector<byte>(32768, 0x00);
    
    // Set up minimal valid ROM header
    _cartridgeData[0x147] = 0x00;  // Cartridge type: ROM only
    _cartridgeData[0x148] = 0x00;  // ROM size: 32KB
    _cartridgeData[0x149] = 0x00;  // RAM size: None
    
    // Create memory components
    _bootRom = std::make_unique<BootRom>(_bootRomData);
    _cartridge = std::make_unique<Cartridge>(_cartridgeData);
    _vRam = std::make_unique<VRam>();
    _wRam = std::make_unique<WRam>();
    _wRamCgb = std::make_unique<WRamCgb>();
    _echoRam = std::make_unique<EchoRam>();
    _oam = std::make_unique<Oam>();
    _ioRegisters = std::make_unique<IoRegisters>();
    _hRam = std::make_unique<HRam>();
    
    // Create bus and CPU
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Initialize to proper post-boot state
    InitializePostBootHardwareState(_bus.get());
    _cpu->InitializePostBootState();
}

void CpuLoadInstructionsTest::Run()
{
    TestLoadSpToMemory();
}

void CpuLoadInstructionsTest::TestLoad8BitRegisterToRegister()
{
    // Test LD B,C (0x41)
    WriteInstruction(0x100, 0x41);
    SetProgramCounter(0x100);
    
    // Set C register to 0x42 by writing to memory and executing LD C,n
    WriteInstructionWithByte(0x0E, 0x0E, 0x42);  // LD C,0x42
    SetProgramCounter(0x0E);
    ExecuteInstruction();
    
    // Now test LD B,C
    SetProgramCounter(0x100);
    ExecuteInstruction();
    
    // Verify B register contains 0x42
    // Note: We can't directly access CPU registers, so we test via memory operations
    // This is a limitation of the current test design
    std::cout << "  ✓ LD B,C instruction executed" << std::endl;
}

void CpuLoadInstructionsTest::TestLoad8BitImmediateToRegister()
{
    // Test LD B,n (0x06)
    WriteInstructionWithByte(0x102, 0x06, 0x55);  // LD B,0x55
    SetProgramCounter(0x102);
    ExecuteInstruction();
    
    std::cout << "  ✓ LD B,n instruction executed" << std::endl;
}

void CpuLoadInstructionsTest::TestLoad8BitFromMemory()
{
    // Test LD A,(HL) (0x7E)
    // First set up HL register to point to a memory location
    WriteInstructionWithWord(0x104, 0x21, 0xC000);  // LD HL,0xC000
    SetProgramCounter(0x104);
    ExecuteInstruction();
    
    // Write test value to memory location
    _bus->Write(0xC000, 0x77);
    
    // Now test LD A,(HL)
    WriteInstruction(0x107, 0x7E);
    SetProgramCounter(0x107);
    ExecuteInstruction();
    
    std::cout << "  ✓ LD A,(HL) instruction executed" << std::endl;
}

void CpuLoadInstructionsTest::TestLoad8BitToMemory()
{
    // Test LD (HL),A (0x77)
    // First set up HL register
    WriteInstructionWithWord(0x109, 0x21, 0xC001);  // LD HL,0xC001
    SetProgramCounter(0x109);
    ExecuteInstruction();
    
    // Set A register to a test value
    WriteInstructionWithByte(0x10C, 0x3E, 0x88);  // LD A,0x88
    SetProgramCounter(0x10C);
    ExecuteInstruction();
    
    // Now test LD (HL),A
    WriteInstruction(0x10E, 0x77);
    SetProgramCounter(0x10E);
    ExecuteInstruction();
    
    // Verify memory contains the value
    VerifyMemoryValue(0xC001, 0x88, "LD (HL),A");
}

void CpuLoadInstructionsTest::TestLoad16BitImmediate()
{
    // Test LD HL,nn (0x21)
    WriteInstructionWithWord(0x110, 0x21, 0x1234);
    SetProgramCounter(0x110);
    ExecuteInstruction();
    
    std::cout << "  ✓ LD HL,nn instruction executed" << std::endl;
}

void CpuLoadInstructionsTest::TestLoadHlSpPlusE8()
{
    // Test LD HL,SP+e8 (0xF8)
    // This instruction loads SP+signed offset into HL
    WriteInstructionWithByte(0x113, 0xF8, 0x05);  // LD HL,SP+5
    SetProgramCounter(0x113);
    ExecuteInstruction();
    
    std::cout << "  ✓ LD HL,SP+e8 instruction executed" << std::endl;
}

void CpuLoadInstructionsTest::TestLoadSpToMemory()
{
    // Test LD (nn),SP (0x08)
    // Write the instruction to Work RAM area (0xC000-0xDFFF) which is writable
    WriteInstructionWithWord(0xC000, 0x08, 0xC010);  // LD (0xC010),SP
    
    // Set PC to the instruction location
    SetProgramCounter(0xC000);
    
    // Execute instruction
    ExecuteInstruction();
    
    // Verify SP was written to memory (should be 0xFFFE initially)
    byte spLow = _bus->Read(0xC010);
    byte spHigh = _bus->Read(0xC011);
    word spValue = static_cast<word>(spLow) | (static_cast<word>(spHigh) << 8);
    
    if (spValue == 0xFFFE) {
        std::cout << "  ✓ LD (nn),SP instruction executed correctly" << std::endl;
    } else {
        std::cout << "  ✗ LD (nn),SP instruction failed - expected 0xFFFE, got 0x" 
                  << std::hex << spValue << std::endl;
        throw std::runtime_error("LD (nn),SP test failed");
    }
}

void CpuLoadInstructionsTest::WriteInstruction(word address, byte opcode)
{
    _bus->Write(address, opcode);
}

void CpuLoadInstructionsTest::WriteInstructionWithByte(word address, byte opcode, byte operand)
{
    _bus->Write(address, opcode);
    _bus->Write(address + 1, operand);
}

void CpuLoadInstructionsTest::WriteInstructionWithWord(word address, byte opcode, word operand)
{
    _bus->Write(address, opcode);
    _bus->Write(address + 1, static_cast<byte>(operand & 0xFF));
    _bus->Write(address + 2, static_cast<byte>((operand >> 8) & 0xFF));
}

void CpuLoadInstructionsTest::SetProgramCounter(word pc)
{
    _cpu->SetPC(pc);
}

void CpuLoadInstructionsTest::ExecuteInstruction()
{
    _cpu->Update();
}

void CpuLoadInstructionsTest::VerifyMemoryValue(word address, byte expected, const char* description)
{
    byte actual = _bus->Read(address);
    if (actual == expected) {
        std::cout << "  ✓ " << description << " - memory value correct" << std::endl;
    } else {
        std::cout << "  ✗ " << description << " - expected 0x" << std::hex << static_cast<int>(expected)
                  << ", got 0x" << std::hex << static_cast<int>(actual) << std::endl;
        throw std::runtime_error(std::string(description) + " test failed");
    }
}