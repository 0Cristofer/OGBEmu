#include "CpuIncDecInstructionsTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

CpuIncDecInstructionsTest::CpuIncDecInstructionsTest() : BaseTest("CPU Inc/Dec Instructions Test")
{
}

void CpuIncDecInstructionsTest::Setup()
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

void CpuIncDecInstructionsTest::Run()
{
    TestIncrement8Bit();
    TestDecrement8Bit();
    TestIncrement16Bit();
    TestDecrement16Bit();
    TestIncrementMemory();
    TestDecrementMemory();
    TestIncrementFlags();
    TestDecrementFlags();
}

void CpuIncDecInstructionsTest::TestIncrement8Bit()
{
    // Test INC B (0x04)
    std::vector<byte> program = {
        0x06, 0x42,        // LD B,0x42
        0x04,              // INC B (should become 0x43)
        0x78,              // LD A,B
        0xEA, 0x00, 0xC0,  // LD (0xC000),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC000, 0x43, "INC B result");
    std::cout << "  ✓ INC B test passed" << std::endl;
}

void CpuIncDecInstructionsTest::TestDecrement8Bit()
{
    // Test DEC C (0x0D)
    std::vector<byte> program = {
        0x0E, 0x42,        // LD C,0x42
        0x0D,              // DEC C (should become 0x41)
        0x79,              // LD A,C
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC001, 0x41, "DEC C result");
    std::cout << "  ✓ DEC C test passed" << std::endl;
}

void CpuIncDecInstructionsTest::TestIncrement16Bit()
{
    // Test INC BC (0x03)
    std::vector<byte> program = {
        0x01, 0xFF, 0x00,  // LD BC,0x00FF
        0x03,              // INC BC (should become 0x0100)
        0x79,              // LD A,C
        0xEA, 0x02, 0xC0,  // LD (0xC002),A
        0x78,              // LD A,B
        0xEA, 0x03, 0xC0,  // LD (0xC003),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC002, 0x00, "INC BC result (C register)");
    VerifyMemoryValue(0xC003, 0x01, "INC BC result (B register)");
    std::cout << "  ✓ INC BC test passed" << std::endl;
}

void CpuIncDecInstructionsTest::TestDecrement16Bit()
{
    // Test DEC DE (0x1B)
    std::vector<byte> program = {
        0x11, 0x00, 0x01,  // LD DE,0x0100
        0x1B,              // DEC DE (should become 0x00FF)
        0x7B,              // LD A,E
        0xEA, 0x04, 0xC0,  // LD (0xC004),A
        0x7A,              // LD A,D
        0xEA, 0x05, 0xC0,  // LD (0xC005),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC004, 0xFF, "DEC DE result (E register)");
    VerifyMemoryValue(0xC005, 0x00, "DEC DE result (D register)");
    std::cout << "  ✓ DEC DE test passed" << std::endl;
}

void CpuIncDecInstructionsTest::TestIncrementMemory()
{
    // Test INC (HL) (0x34)
    std::vector<byte> program = {
        0x21, 0x00, 0xC0,  // LD HL,0xC000
        0x36, 0x7F,        // LD (HL),0x7F
        0x34,              // INC (HL) (should become 0x80)
        0x7E,              // LD A,(HL)
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC000, 0x80, "INC (HL) memory result");
    VerifyMemoryValue(0xC001, 0x80, "INC (HL) loaded result");
    std::cout << "  ✓ INC (HL) test passed" << std::endl;
}

void CpuIncDecInstructionsTest::TestDecrementMemory()
{
    // Test DEC (HL) (0x35)
    std::vector<byte> program = {
        0x21, 0x00, 0xC0,  // LD HL,0xC000
        0x36, 0x01,        // LD (HL),0x01
        0x35,              // DEC (HL) (should become 0x00)
        0x7E,              // LD A,(HL)
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC000, 0x00, "DEC (HL) memory result");
    VerifyMemoryValue(0xC001, 0x00, "DEC (HL) loaded result");
    std::cout << "  ✓ DEC (HL) test passed" << std::endl;
}

void CpuIncDecInstructionsTest::TestIncrementFlags()
{
    // Test INC A with overflow (sets Z flag)
    std::vector<byte> program = {
        0x3E, 0xFF,        // LD A,0xFF
        0x3C,              // INC A (should become 0x00, set Z flag)
        0xEA, 0x06, 0xC0,  // LD (0xC006),A
        
        // Test half-carry flag with INC
        0x3E, 0x0F,        // LD A,0x0F
        0x3C,              // INC A (should become 0x10, set H flag)
        0xEA, 0x07, 0xC0,  // LD (0xC007),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC006, 0x00, "INC A overflow result");
    VerifyMemoryValue(0xC007, 0x10, "INC A half-carry result");
    std::cout << "  ✓ INC flag tests passed" << std::endl;
}

void CpuIncDecInstructionsTest::TestDecrementFlags()
{
    // Test DEC A with underflow (sets Z flag)
    std::vector<byte> program = {
        0x3E, 0x01,        // LD A,0x01
        0x3D,              // DEC A (should become 0x00, set Z flag)
        0xEA, 0x08, 0xC0,  // LD (0xC008),A
        
        // Test half-carry flag with DEC
        0x3E, 0x10,        // LD A,0x10
        0x3D,              // DEC A (should become 0x0F, set H flag)
        0xEA, 0x09, 0xC0,  // LD (0xC009),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC008, 0x00, "DEC A underflow result");
    VerifyMemoryValue(0xC009, 0x0F, "DEC A half-carry result");
    std::cout << "  ✓ DEC flag tests passed" << std::endl;
}

void CpuIncDecInstructionsTest::WriteProgramToCartridge(const std::vector<byte>& program)
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

void CpuIncDecInstructionsTest::WriteProgram(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size(); ++i) {
        _bus->Write(0x100 + static_cast<word>(i), program[i]);
    }
}

void CpuIncDecInstructionsTest::ExecuteProgram(int steps)
{
    for (int i = 0; i < steps; ++i) {
        _cpu->Update();
    }
}

void CpuIncDecInstructionsTest::VerifyMemoryValue(word address, byte expected, const char* description)
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

void CpuIncDecInstructionsTest::VerifyRegisterValue(byte expected, const char* description)
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

void CpuIncDecInstructionsTest::VerifyFlag(bool expected, bool actual, const char* flagName)
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