#include "CpuPrefixInstructionsTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

CpuPrefixInstructionsTest::CpuPrefixInstructionsTest() : BaseTest("CPU Prefix Instructions Test")
{
}

void CpuPrefixInstructionsTest::Setup()
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

void CpuPrefixInstructionsTest::Run()
{
    TestRotateLeftCircular();
    TestRotateRightCircular();
    TestRotateLeft();
    TestRotateRight();
    TestShiftLeftArithmetic();
    TestShiftRightArithmetic();
    TestShiftRightLogical();
    TestSwap();
    TestBitOperations();
    TestSetOperations();
    TestResetOperations();
}

void CpuPrefixInstructionsTest::TestRotateLeftCircular()
{
    // Test RLC B (0xCB 0x00) - Rotate left circular on register B
    std::vector<byte> program = {
        0x06, 0x85,        // LD B,0x85 (10000101)
        0xCB, 0x00,        // RLC B (should become 00001011 = 0x0B, carry = 1)
        0x78,              // LD A,B
        0xEA, 0x00, 0xC0,  // LD (0xC000),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC000, 0x0B, "RLC B result");
    std::cout << "  ✓ RLC B test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestRotateRightCircular()
{
    // Test RRC C (0xCB 0x09) - Rotate right circular on register C
    std::vector<byte> program = {
        0x0E, 0x85,        // LD C,0x85 (10000101)
        0xCB, 0x09,        // RRC C (should become 11000010 = 0xC2, carry = 1)
        0x79,              // LD A,C
        0xEA, 0x01, 0xC0,  // LD (0xC001),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC001, 0xC2, "RRC C result");
    std::cout << "  ✓ RRC C test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestRotateLeft()
{
    // Test RL D (0xCB 0x12) - Rotate left through carry
    std::vector<byte> program = {
        0x16, 0x42,        // LD D,0x42 (01000010)
        0x37,              // SCF (set carry flag)
        0xCB, 0x12,        // RL D (should become 10000101 = 0x85)
        0x7A,              // LD A,D
        0xEA, 0x02, 0xC0,  // LD (0xC002),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(20);
    
    VerifyMemoryValue(0xC002, 0x85, "RL D result");
    std::cout << "  ✓ RL D test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestRotateRight()
{
    // Test RR E (0xCB 0x1B) - Rotate right through carry
    std::vector<byte> program = {
        0x1E, 0x42,        // LD E,0x42 (01000010)
        0x37,              // SCF (set carry flag)
        0xCB, 0x1B,        // RR E (should become 10100001 = 0xA1)
        0x7B,              // LD A,E
        0xEA, 0x03, 0xC0,  // LD (0xC003),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(20);
    
    VerifyMemoryValue(0xC003, 0xA1, "RR E result");
    std::cout << "  ✓ RR E test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestShiftLeftArithmetic()
{
    // Test SLA H (0xCB 0x24) - Shift left arithmetic
    std::vector<byte> program = {
        0x26, 0x42,        // LD H,0x42 (01000010)
        0xCB, 0x24,        // SLA H (should become 10000100 = 0x84)
        0x7C,              // LD A,H
        0xEA, 0x04, 0xC0,  // LD (0xC004),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC004, 0x84, "SLA H result");
    std::cout << "  ✓ SLA H test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestShiftRightArithmetic()
{
    // Test SRA L (0xCB 0x2D) - Shift right arithmetic (preserves sign bit)
    std::vector<byte> program = {
        0x2E, 0x84,        // LD L,0x84 (10000100)
        0xCB, 0x2D,        // SRA L (should become 11000010 = 0xC2)
        0x7D,              // LD A,L
        0xEA, 0x05, 0xC0,  // LD (0xC005),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC005, 0xC2, "SRA L result");
    std::cout << "  ✓ SRA L test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestShiftRightLogical()
{
    // Test SRL A (0xCB 0x3F) - Shift right logical
    std::vector<byte> program = {
        0x3E, 0x84,        // LD A,0x84 (10000100)
        0xCB, 0x3F,        // SRL A (should become 01000010 = 0x42)
        0xEA, 0x06, 0xC0,  // LD (0xC006),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(12);
    
    VerifyMemoryValue(0xC006, 0x42, "SRL A result");
    std::cout << "  ✓ SRL A test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestSwap()
{
    // Test SWAP B (0xCB 0x30) - Swap nibbles
    std::vector<byte> program = {
        0x06, 0x3C,        // LD B,0x3C (00111100)
        0xCB, 0x30,        // SWAP B (should become 11000011 = 0xC3)
        0x78,              // LD A,B
        0xEA, 0x07, 0xC0,  // LD (0xC007),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC007, 0xC3, "SWAP B result");
    std::cout << "  ✓ SWAP B test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestBitOperations()
{
    // Test BIT 7,C (0xCB 0x79) - Test bit 7 of register C
    std::vector<byte> program = {
        0x0E, 0x80,        // LD C,0x80 (bit 7 set)
        0xCB, 0x79,        // BIT 7,C (should clear Z flag since bit is set)
        0x06, 0x00,        // LD B,0x00
        0x0E, 0x7F,        // LD C,0x7F (bit 7 clear)
        0xCB, 0x79,        // BIT 7,C (should set Z flag since bit is clear)
        0x06, 0x01,        // LD B,0x01
        0x78,              // LD A,B
        0xEA, 0x08, 0xC0,  // LD (0xC008),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(25);
    
    VerifyMemoryValue(0xC008, 0x01, "BIT test result");
    std::cout << "  ✓ BIT operation test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestSetOperations()
{
    // Test SET 3,D (0xCB 0xDA) - Set bit 3 of register D
    std::vector<byte> program = {
        0x16, 0x00,        // LD D,0x00 (00000000)
        0xCB, 0xDA,        // SET 3,D (should become 00001000 = 0x08)
        0x7A,              // LD A,D
        0xEA, 0x09, 0xC0,  // LD (0xC009),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC009, 0x08, "SET 3,D result");
    std::cout << "  ✓ SET operation test passed" << std::endl;
}

void CpuPrefixInstructionsTest::TestResetOperations()
{
    // Test RES 5,E (0xCB 0xAB) - Reset bit 5 of register E
    std::vector<byte> program = {
        0x1E, 0xFF,        // LD E,0xFF (11111111)
        0xCB, 0xAB,        // RES 5,E (should become 11011111 = 0xDF)
        0x7B,              // LD A,E
        0xEA, 0x0A, 0xC0,  // LD (0xC00A),A
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(15);
    
    VerifyMemoryValue(0xC00A, 0xDF, "RES 5,E result");
    std::cout << "  ✓ RES operation test passed" << std::endl;
}

void CpuPrefixInstructionsTest::WriteProgramToCartridge(const std::vector<byte>& program)
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

void CpuPrefixInstructionsTest::WriteProgram(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size(); ++i) {
        _bus->Write(0x100 + static_cast<word>(i), program[i]);
    }
}

void CpuPrefixInstructionsTest::ExecuteProgram(int steps)
{
    for (int i = 0; i < steps; ++i) {
        _cpu->Update();
    }
}

void CpuPrefixInstructionsTest::VerifyMemoryValue(word address, byte expected, const char* description)
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

void CpuPrefixInstructionsTest::VerifyRegisterValue(byte expected, const char* description)
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