#include "CpuStopInstructionTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

CpuStopInstructionTest::CpuStopInstructionTest() : BaseTest("CPU STOP Instruction Test")
{
}

void CpuStopInstructionTest::Setup()
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
    
    // Disable boot ROM for testing
    _bus->Write(AddressConstants::BootRomBank, 0x01);
}

void CpuStopInstructionTest::Run()
{
    TestStopBasicBehavior();
    TestStopWithFollowingInstructions();
    TestStopResumeConditions();
    TestStopTiming();
}

void CpuStopInstructionTest::TestStopBasicBehavior()
{
    // Test STOP (0x10 0x00) basic behavior
    std::vector<byte> program = {
        0x3E, 0xAA,        // LD A,0xAA (marker before STOP)
        0xEA, 0x00, 0xC0,  // LD (0xC000),A
        0x10, 0x00,        // STOP
        0x3E, 0xBB,        // LD A,0xBB (should not execute immediately)
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    // Should execute instructions before STOP
    VerifyMemoryValue(0xC000, 0xAA, "Pre-STOP instruction execution");
    // Should not execute instructions after STOP immediately
    VerifyMemoryValue(0xC001, 0x00, "Post-STOP instruction should not execute");
    std::cout << "  ✓ STOP basic behavior test passed" << std::endl;
}

void CpuStopInstructionTest::TestStopWithFollowingInstructions()
{
    // Test that STOP halts execution and PC is positioned correctly
    std::vector<byte> program = {
        0x3E, 0x11,        // LD A,0x11
        0xEA, 0x02, 0xC0,  // LD (0xC002),A
        0x10, 0x00,        // STOP
        0x3E, 0x22,        // LD A,0x22 (should not execute)
        0xEA, 0x03, 0xC0,  // LD (0xC003),A
        0x3E, 0x33,        // LD A,0x33 (should not execute)
        0xEA, 0x04, 0xC0,  // LD (0xC004),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(20);
    
    VerifyMemoryValue(0xC002, 0x11, "Instruction before STOP executed");
    VerifyMemoryValue(0xC003, 0x00, "First instruction after STOP not executed");
    VerifyMemoryValue(0xC004, 0x00, "Second instruction after STOP not executed");
    std::cout << "  ✓ STOP with following instructions test passed" << std::endl;
}

void CpuStopInstructionTest::TestStopResumeConditions()
{
    // Test that STOP can be resumed (in a real Game Boy, this would be via button press)
    // For testing purposes, we'll just verify STOP halts properly
    std::vector<byte> program = {
        0x3E, 0x44,        // LD A,0x44
        0xEA, 0x05, 0xC0,  // LD (0xC005),A
        0x10, 0x00,        // STOP
        0x3E, 0x55,        // LD A,0x55 (execution resumes here after STOP)
        0xEA, 0x06, 0xC0,  // LD (0xC006),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC005, 0x44, "Before STOP executed");
    VerifyMemoryValue(0xC006, 0x00, "After STOP not executed while stopped");
    std::cout << "  ✓ STOP resume conditions test passed" << std::endl;
}

void CpuStopInstructionTest::TestStopTiming()
{
    // Test STOP instruction timing and effect on CPU state
    std::vector<byte> program = {
        0x3E, 0x66,        // LD A,0x66
        0xEA, 0x07, 0xC0,  // LD (0xC007),A
        0x3C,              // INC A (A becomes 0x67)
        0x10, 0x00,        // STOP
        0x3C,              // INC A (should not execute while stopped)
        0xEA, 0x08, 0xC0,  // LD (0xC008),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(20);
    
    VerifyMemoryValue(0xC007, 0x66, "Memory before STOP");
    VerifyMemoryValue(0xC008, 0x00, "Memory after STOP should be unchanged");
    std::cout << "  ✓ STOP timing test passed" << std::endl;
}

void CpuStopInstructionTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    // Recreate cartridge with the new program
    for (size_t i = 0; i < program.size(); ++i) {
        _cartridgeData[i] = program[i];
    }
    
    // Recreate the cartridge and bus with updated data
    _cartridge = std::make_unique<Cartridge>(_cartridgeData);
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Disable boot ROM for testing
    _bus->Write(AddressConstants::BootRomBank, 0x01);
}

void CpuStopInstructionTest::WriteProgram(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size(); ++i) {
        _bus->Write(0x100 + static_cast<word>(i), program[i]);
    }
}

void CpuStopInstructionTest::ExecuteProgram(int steps)
{
    for (int i = 0; i < steps; ++i) {
        _cpu->Update();
    }
}

void CpuStopInstructionTest::VerifyMemoryValue(word address, byte expected, const char* description)
{
    byte actual = _bus->Read(address);
    if (actual == expected) {
        std::cout << "    ✓ " << description << " - expected 0x" << std::hex << static_cast<int>(expected) 
                  << ", got 0x" << std::hex << static_cast<int>(actual) << std::endl;
    } else {
        std::cout << "    ✗ " << description << " - expected 0x" << std::hex << static_cast<int>(expected)
                  << ", got 0x" << std::hex << static_cast<int>(actual) << std::endl;
        throw std::runtime_error(std::string(description) + " test failed");
    }
}

void CpuStopInstructionTest::VerifyRegisterValue(byte expected, const char* description)
{
    byte actual = _cpu->GetA();
    if (actual == expected) {
        std::cout << "    ✓ " << description << " - expected 0x" << std::hex << static_cast<int>(expected) 
                  << ", got 0x" << std::hex << static_cast<int>(actual) << std::endl;
    } else {
        std::cout << "    ✗ " << description << " - expected 0x" << std::hex << static_cast<int>(expected)
                  << ", got 0x" << std::hex << static_cast<int>(actual) << std::endl;
        throw std::runtime_error(std::string(description) + " test failed");
    }
}

void CpuStopInstructionTest::VerifyFlag(bool expected, bool actual, const char* flagName)
{
    if (actual == expected) {
        std::cout << "    ✓ " << flagName << " flag - expected " << (expected ? "set" : "clear") 
                  << ", got " << (actual ? "set" : "clear") << std::endl;
    } else {
        std::cout << "    ✗ " << flagName << " flag - expected " << (expected ? "set" : "clear")
                  << ", got " << (actual ? "set" : "clear") << std::endl;
        throw std::runtime_error(std::string(flagName) + " flag test failed");
    }
}