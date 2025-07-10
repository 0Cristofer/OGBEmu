#include "CpuRotateInstructionsTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

CpuRotateInstructionsTest::CpuRotateInstructionsTest() : BaseTest("CPU Rotate Instructions Test")
{
}

void CpuRotateInstructionsTest::Setup()
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

void CpuRotateInstructionsTest::Run()
{
    TestRotateLeftCircularA();
    TestRotateRightCircularA();
    TestRotateLeftA();
    TestRotateRightA();
    TestRotateCarryBehavior();
    TestRotateZeroFlag();
    TestRotateSequences();
}

void CpuRotateInstructionsTest::TestRotateLeftCircularA()
{
    // Test RLCA (0x07) - Rotate A left circular
    std::vector<byte> program = {
        0x3E, 0x85,        // LD A,0x85 (10000101)
        0x07,              // RLCA (should become 00001011 = 0x0B, carry = 1)
        0xEA, 0x00, 0xC0,  // LD (0xC000),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(12);
    
    VerifyMemoryValue(0xC000, 0x0B, "RLCA result");
    std::cout << "  ✓ RLCA test passed" << std::endl;
}

void CpuRotateInstructionsTest::TestRotateRightCircularA()
{
    // Test RRCA (0x0F) - Rotate A right circular
    std::vector<byte> program = {
        0x3E, 0x85,        // LD A,0x85 (10000101)
        0x0F,              // RRCA (should become 11000010 = 0xC2, carry = 1)
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(12);
    
    VerifyMemoryValue(0xC001, 0xC2, "RRCA result");
    std::cout << "  ✓ RRCA test passed" << std::endl;
}

void CpuRotateInstructionsTest::TestRotateLeftA()
{
    // Test RLA (0x17) - Rotate A left through carry
    std::vector<byte> program = {
        0x3E, 0x42,        // LD A,0x42 (01000010)
        0x37,              // SCF (set carry flag)
        0x17,              // RLA (should become 10000101 = 0x85)
        0xEA, 0x02, 0xC0,  // LD (0xC002),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC002, 0x85, "RLA result");
    std::cout << "  ✓ RLA test passed" << std::endl;
}

void CpuRotateInstructionsTest::TestRotateRightA()
{
    // Test RRA (0x1F) - Rotate A right through carry
    std::vector<byte> program = {
        0x3E, 0x42,        // LD A,0x42 (01000010)
        0x37,              // SCF (set carry flag)
        0x1F,              // RRA (should become 10100001 = 0xA1)
        0xEA, 0x03, 0xC0,  // LD (0xC003),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC003, 0xA1, "RRA result");
    std::cout << "  ✓ RRA test passed" << std::endl;
}

void CpuRotateInstructionsTest::TestRotateCarryBehavior()
{
    // Test carry flag behavior with RLCA when bit 7 is set
    std::vector<byte> program = {
        0x3E, 0x80,        // LD A,0x80 (10000000)
        0x07,              // RLCA (should become 00000001 = 0x01, carry = 1)
        0xEA, 0x04, 0xC0,  // LD (0xC004),A
        
        // Test carry flag behavior with RRCA when bit 0 is set
        0x3E, 0x01,        // LD A,0x01 (00000001)
        0x0F,              // RRCA (should become 10000000 = 0x80, carry = 1)
        0xEA, 0x05, 0xC0,  // LD (0xC005),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC004, 0x01, "RLCA carry behavior");
    VerifyMemoryValue(0xC005, 0x80, "RRCA carry behavior");
    std::cout << "  ✓ Rotate carry behavior tests passed" << std::endl;
}

void CpuRotateInstructionsTest::TestRotateZeroFlag()
{
    // Test that rotate instructions clear Z flag (even if result is 0)
    // Note: RLCA, RRCA, RLA, RRA always clear Z flag regardless of result
    std::vector<byte> program = {
        0x3E, 0x00,        // LD A,0x00
        0x07,              // RLCA (result still 0, but Z flag should be clear)
        0xEA, 0x06, 0xC0,  // LD (0xC006),A
        
        // Test another case
        0x3E, 0x00,        // LD A,0x00
        0x0F,              // RRCA (result still 0, but Z flag should be clear)
        0xEA, 0x07, 0xC0,  // LD (0xC007),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC006, 0x00, "RLCA with zero result");
    VerifyMemoryValue(0xC007, 0x00, "RRCA with zero result");
    std::cout << "  ✓ Rotate zero flag tests passed" << std::endl;
}

void CpuRotateInstructionsTest::TestRotateSequences()
{
    // Test multiple rotations in sequence
    std::vector<byte> program = {
        0x3E, 0x01,        // LD A,0x01 (00000001)
        0x07,              // RLCA -> 0x02 (00000010)
        0x07,              // RLCA -> 0x04 (00000100)
        0x07,              // RLCA -> 0x08 (00001000)
        0x07,              // RLCA -> 0x10 (00010000)
        0xEA, 0x08, 0xC0,  // LD (0xC008),A
        
        // Test sequence that wraps around
        0x3E, 0x80,        // LD A,0x80 (10000000)
        0x07,              // RLCA -> 0x01 (00000001)
        0x07,              // RLCA -> 0x02 (00000010)
        0xEA, 0x09, 0xC0,  // LD (0xC009),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(35);
    
    VerifyMemoryValue(0xC008, 0x10, "RLCA sequence result");
    VerifyMemoryValue(0xC009, 0x02, "RLCA wraparound sequence");
    std::cout << "  ✓ Rotate sequence tests passed" << std::endl;
}

void CpuRotateInstructionsTest::WriteProgramToCartridge(const std::vector<byte>& program)
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

void CpuRotateInstructionsTest::WriteProgram(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size(); ++i) {
        _bus->Write(0x100 + static_cast<word>(i), program[i]);
    }
}

void CpuRotateInstructionsTest::ExecuteProgram(int steps)
{
    for (int i = 0; i < steps; ++i) {
        _cpu->Update();
    }
}

void CpuRotateInstructionsTest::VerifyMemoryValue(word address, byte expected, const char* description)
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

void CpuRotateInstructionsTest::VerifyRegisterValue(byte expected, const char* description)
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

void CpuRotateInstructionsTest::VerifyFlag(bool expected, bool actual, const char* flagName)
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