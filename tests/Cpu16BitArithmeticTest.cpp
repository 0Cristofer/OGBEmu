#include "Cpu16BitArithmeticTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

Cpu16BitArithmeticTest::Cpu16BitArithmeticTest() : BaseTest("CPU 16-Bit Arithmetic Test")
{
}

void Cpu16BitArithmeticTest::Setup()
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

void Cpu16BitArithmeticTest::Run()
{
    TestAddHlBc();
    TestAddHlDe();
    TestAddHlHl();
    TestAddHlSp();
    TestAddHlCarryBehavior();
    TestAddHlFlagBehavior();
    TestAddHlOverflow();
    TestAddSpE8();
}

void Cpu16BitArithmeticTest::TestAddHlBc()
{
    // Test ADD HL,BC (0x09)
    std::vector<byte> program = {
        0x21, 0x00, 0x10,  // LD HL,0x1000
        0x01, 0x34, 0x12,  // LD BC,0x1234
        0x09,              // ADD HL,BC (0x1000 + 0x1234 = 0x2234)
        0x7C,              // LD A,H
        0xEA, 0x00, 0xC0,  // LD (0xC000),A
        0x7D,              // LD A,L
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(30);
    
    VerifyMemoryValue(0xC000, 0x22, "ADD HL,BC result (H register)");
    VerifyMemoryValue(0xC001, 0x34, "ADD HL,BC result (L register)");
    std::cout << "  ✓ ADD HL,BC test passed" << std::endl;
}

void Cpu16BitArithmeticTest::TestAddHlDe()
{
    // Test ADD HL,DE (0x19)
    std::vector<byte> program = {
        0x21, 0x00, 0x20,  // LD HL,0x2000
        0x11, 0x56, 0x78,  // LD DE,0x7856
        0x19,              // ADD HL,DE (0x2000 + 0x7856 = 0x9856)
        0x7C,              // LD A,H
        0xEA, 0x02, 0xC0,  // LD (0xC002),A
        0x7D,              // LD A,L
        0xEA, 0x03, 0xC0,  // LD (0xC003),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(30);
    
    VerifyMemoryValue(0xC002, 0x98, "ADD HL,DE result (H register)");
    VerifyMemoryValue(0xC003, 0x56, "ADD HL,DE result (L register)");
    std::cout << "  ✓ ADD HL,DE test passed" << std::endl;
}

void Cpu16BitArithmeticTest::TestAddHlHl()
{
    // Test ADD HL,HL (0x29) - effectively doubling HL
    std::vector<byte> program = {
        0x21, 0x00, 0x80,  // LD HL,0x8000
        0x29,              // ADD HL,HL (0x8000 + 0x8000 = 0x0000 with carry)
        0x7C,              // LD A,H
        0xEA, 0x04, 0xC0,  // LD (0xC004),A
        0x7D,              // LD A,L
        0xEA, 0x05, 0xC0,  // LD (0xC005),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC004, 0x00, "ADD HL,HL result (H register)");
    VerifyMemoryValue(0xC005, 0x00, "ADD HL,HL result (L register)");
    std::cout << "  ✓ ADD HL,HL test passed" << std::endl;
}

void Cpu16BitArithmeticTest::TestAddHlSp()
{
    // Test ADD HL,SP (0x39)
    std::vector<byte> program = {
        0x21, 0x00, 0x30,  // LD HL,0x3000
        0x31, 0x00, 0x40,  // LD SP,0x4000
        0x39,              // ADD HL,SP (0x3000 + 0x4000 = 0x7000)
        0x7C,              // LD A,H
        0xEA, 0x06, 0xC0,  // LD (0xC006),A
        0x7D,              // LD A,L
        0xEA, 0x07, 0xC0,  // LD (0xC007),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(30);
    
    VerifyMemoryValue(0xC006, 0x70, "ADD HL,SP result (H register)");
    VerifyMemoryValue(0xC007, 0x00, "ADD HL,SP result (L register)");
    std::cout << "  ✓ ADD HL,SP test passed" << std::endl;
}

void Cpu16BitArithmeticTest::TestAddHlCarryBehavior()
{
    // Test ADD HL with carry flag behavior
    std::vector<byte> program = {
        0x21, 0xFF, 0xFF,  // LD HL,0xFFFF
        0x01, 0x01, 0x00,  // LD BC,0x0001
        0x09,              // ADD HL,BC (0xFFFF + 0x0001 = 0x0000 with carry)
        0x7C,              // LD A,H
        0xEA, 0x08, 0xC0,  // LD (0xC008),A
        0x7D,              // LD A,L
        0xEA, 0x09, 0xC0,  // LD (0xC009),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(30);
    
    VerifyMemoryValue(0xC008, 0x00, "ADD HL carry behavior (H register)");
    VerifyMemoryValue(0xC009, 0x00, "ADD HL carry behavior (L register)");
    std::cout << "  ✓ ADD HL carry behavior test passed" << std::endl;
}

void Cpu16BitArithmeticTest::TestAddHlFlagBehavior()
{
    // Test that ADD HL only affects N, H, C flags (preserves Z)
    std::vector<byte> program = {
        0x3E, 0x00,        // LD A,0x00
        0xA7,              // AND A (sets Z flag)
        0x21, 0x00, 0x01,  // LD HL,0x0100
        0x01, 0x00, 0x01,  // LD BC,0x0100
        0x09,              // ADD HL,BC (should preserve Z flag)
        0x7C,              // LD A,H
        0xEA, 0x0A, 0xC0,  // LD (0xC00A),A
        0x7D,              // LD A,L
        0xEA, 0x0B, 0xC0,  // LD (0xC00B),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(35);
    
    VerifyMemoryValue(0xC00A, 0x02, "ADD HL flag behavior (H register)");
    VerifyMemoryValue(0xC00B, 0x00, "ADD HL flag behavior (L register)");
    std::cout << "  ✓ ADD HL flag behavior test passed" << std::endl;
}

void Cpu16BitArithmeticTest::TestAddHlOverflow()
{
    // Test ADD HL with various overflow scenarios
    std::vector<byte> program = {
        0x21, 0x00, 0x7F,  // LD HL,0x7F00
        0x01, 0xFF, 0x00,  // LD BC,0x00FF
        0x09,              // ADD HL,BC (0x7F00 + 0x00FF = 0x7FFF)
        0x7C,              // LD A,H
        0xEA, 0x0C, 0xC0,  // LD (0xC00C),A
        0x7D,              // LD A,L
        0xEA, 0x0D, 0xC0,  // LD (0xC00D),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(30);
    
    VerifyMemoryValue(0xC00C, 0x7F, "ADD HL overflow (H register)");
    VerifyMemoryValue(0xC00D, 0xFF, "ADD HL overflow (L register)");
    std::cout << "  ✓ ADD HL overflow test passed" << std::endl;
}

void Cpu16BitArithmeticTest::TestAddSpE8()
{
    // Test ADD SP,e8 (0xE8) - add signed 8-bit to SP
    std::vector<byte> program = {
        0x31, 0x00, 0x50,  // LD SP,0x5000
        0xE8, 0x10,        // ADD SP,+16 (0x5000 + 16 = 0x5010)
        0xF8, 0x00,        // LD HL,SP+0 (copy SP to HL)
        0x7C,              // LD A,H
        0xEA, 0x0E, 0xC0,  // LD (0xC00E),A
        0x7D,              // LD A,L
        0xEA, 0x0F, 0xC0,  // LD (0xC00F),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(35);
    
    VerifyMemoryValue(0xC00E, 0x50, "ADD SP,e8 result (H from HL)");
    VerifyMemoryValue(0xC00F, 0x10, "ADD SP,e8 result (L from HL)");
    std::cout << "  ✓ ADD SP,e8 test passed" << std::endl;
}

void Cpu16BitArithmeticTest::WriteProgramToCartridge(const std::vector<byte>& program)
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

void Cpu16BitArithmeticTest::WriteProgram(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size(); ++i) {
        _bus->Write(0x100 + static_cast<word>(i), program[i]);
    }
}

void Cpu16BitArithmeticTest::ExecuteProgram(int steps)
{
    for (int i = 0; i < steps; ++i) {
        _cpu->Update();
    }
}

void Cpu16BitArithmeticTest::VerifyMemoryValue(word address, byte expected, const char* description)
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

void Cpu16BitArithmeticTest::VerifyMemoryValue16(word address, word expected, const char* description)
{
    byte low = _bus->Read(address);
    byte high = _bus->Read(address + 1);
    word actual = (static_cast<word>(high) << 8) | low;
    
    if (actual == expected) {
        std::cout << "    ✓ " << description << " - expected 0x" << std::hex << expected 
                  << ", got 0x" << std::hex << actual << std::endl;
    } else {
        std::cout << "    ✗ " << description << " - expected 0x" << std::hex << expected
                  << ", got 0x" << std::hex << actual << std::endl;
        throw std::runtime_error(std::string(description) + " test failed");
    }
}

void Cpu16BitArithmeticTest::VerifyRegisterValue(byte expected, const char* description)
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

void Cpu16BitArithmeticTest::VerifyFlag(bool expected, bool actual, const char* flagName)
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