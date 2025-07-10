#include "CpuDaaInstructionTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

CpuDaaInstructionTest::CpuDaaInstructionTest() : BaseTest("CPU DAA Instruction Test")
{
}

void CpuDaaInstructionTest::Setup()
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

void CpuDaaInstructionTest::Run()
{
    TestDaaAfterAddition();
    TestDaaAfterSubtraction();
    TestDaaWithCarryFlag();
    TestDaaWithHalfCarryFlag();
    TestDaaEdgeCases();
    TestDaaZeroFlag();
    TestDaaDecimalArithmetic();
}

void CpuDaaInstructionTest::TestDaaAfterAddition()
{
    // Test DAA (0x27) after binary addition that needs BCD correction
    std::vector<byte> program = {
        0x3E, 0x09,        // LD A,0x09 (BCD 9)
        0xC6, 0x01,        // ADD A,0x01 (9 + 1 = 0x0A in binary)
        0x27,              // DAA (should correct to 0x10 = BCD 10)
        0xEA, 0x00, 0xC0,  // LD (0xC000),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC000, 0x10, "DAA after addition");
    std::cout << "  ✓ DAA after addition test passed" << std::endl;
}

void CpuDaaInstructionTest::TestDaaAfterSubtraction()
{
    // Test DAA after subtraction with N flag set
    std::vector<byte> program = {
        0x3E, 0x10,        // LD A,0x10 (BCD 10)
        0xD6, 0x01,        // SUB A,0x01 (should give 0x0F, needs BCD correction to 0x09)
        0x27,              // DAA (should correct to 0x09 = BCD 9)
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC001, 0x09, "DAA after subtraction");
    std::cout << "  ✓ DAA after subtraction test passed" << std::endl;
}

void CpuDaaInstructionTest::TestDaaWithCarryFlag()
{
    // Test DAA when carry flag is set
    std::vector<byte> program = {
        0x3E, 0x99,        // LD A,0x99 (BCD 99)
        0xC6, 0x01,        // ADD A,0x01 (99 + 1 = 0x9A, sets carry)
        0x27,              // DAA (should correct to 0x00 with carry set = BCD 100)
        0xEA, 0x02, 0xC0,  // LD (0xC002),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC002, 0x00, "DAA with carry flag");
    std::cout << "  ✓ DAA with carry flag test passed" << std::endl;
}

void CpuDaaInstructionTest::TestDaaWithHalfCarryFlag()
{
    // Test DAA when half-carry flag is set
    std::vector<byte> program = {
        0x3E, 0x0F,        // LD A,0x0F 
        0xC6, 0x01,        // ADD A,0x01 (0x0F + 0x01 = 0x10, sets half-carry)
        0x27,              // DAA (should correct considering half-carry)
        0xEA, 0x03, 0xC0,  // LD (0xC003),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC003, 0x16, "DAA with half-carry flag");
    std::cout << "  ✓ DAA with half-carry flag test passed" << std::endl;
}

void CpuDaaInstructionTest::TestDaaEdgeCases()
{
    // Test DAA with no correction needed
    std::vector<byte> program = {
        0x3E, 0x05,        // LD A,0x05 (valid BCD)
        0xC6, 0x03,        // ADD A,0x03 (5 + 3 = 0x08, valid BCD)
        0x27,              // DAA (should remain 0x08)
        0xEA, 0x04, 0xC0,  // LD (0xC004),A
        
        // Test DAA with A = 0x00
        0x3E, 0x00,        // LD A,0x00
        0x27,              // DAA (should remain 0x00)
        0xEA, 0x05, 0xC0,  // LD (0xC005),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC004, 0x08, "DAA no correction needed");
    VerifyMemoryValue(0xC005, 0x00, "DAA with zero");
    std::cout << "  ✓ DAA edge cases test passed" << std::endl;
}

void CpuDaaInstructionTest::TestDaaZeroFlag()
{
    // Test that DAA sets zero flag when result is 0
    std::vector<byte> program = {
        0x3E, 0x99,        // LD A,0x99
        0xC6, 0x01,        // ADD A,0x01 (creates 0x9A with carry)
        0x27,              // DAA (should become 0x00 with carry, Z flag set)
        0xEA, 0x06, 0xC0,  // LD (0xC006),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC006, 0x00, "DAA zero flag result");
    std::cout << "  ✓ DAA zero flag test passed" << std::endl;
}

void CpuDaaInstructionTest::TestDaaDecimalArithmetic()
{
    // Test a sequence of decimal arithmetic using DAA
    std::vector<byte> program = {
        0x3E, 0x25,        // LD A,0x25 (BCD 25)
        0xC6, 0x17,        // ADD A,0x17 (25 + 17 in BCD)
        0x27,              // DAA (should give 0x42 = BCD 42)
        0xEA, 0x07, 0xC0,  // LD (0xC007),A
        
        // Test another decimal operation
        0x3E, 0x58,        // LD A,0x58 (BCD 58)
        0xC6, 0x37,        // ADD A,0x37 (58 + 37 in BCD)
        0x27,              // DAA (should give 0x95 = BCD 95)
        0xEA, 0x08, 0xC0,  // LD (0xC008),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(30);
    
    VerifyMemoryValue(0xC007, 0x42, "DAA decimal arithmetic 25+17");
    VerifyMemoryValue(0xC008, 0x95, "DAA decimal arithmetic 58+37");
    std::cout << "  ✓ DAA decimal arithmetic test passed" << std::endl;
}

void CpuDaaInstructionTest::WriteProgramToCartridge(const std::vector<byte>& program)
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

void CpuDaaInstructionTest::WriteProgram(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size(); ++i) {
        _bus->Write(0x100 + static_cast<word>(i), program[i]);
    }
}

void CpuDaaInstructionTest::ExecuteProgram(int steps)
{
    for (int i = 0; i < steps; ++i) {
        _cpu->Update();
    }
}

void CpuDaaInstructionTest::VerifyMemoryValue(word address, byte expected, const char* description)
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

void CpuDaaInstructionTest::VerifyRegisterValue(byte expected, const char* description)
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

void CpuDaaInstructionTest::VerifyFlag(bool expected, bool actual, const char* flagName)
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