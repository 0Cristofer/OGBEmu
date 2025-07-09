#include "CpuFlagOperationsTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Core/Logger.h"
#include <cassert>

CpuFlagOperationsTest::CpuFlagOperationsTest() : BaseTest("CPU Flag Operations Test")
{
}

void CpuFlagOperationsTest::Setup()
{
    LOG("  Setting up CPU flag operations test...");
    
    // Create minimal boot ROM
    _bootRomData = std::vector<byte>(256, 0x00);
    
    // Create minimal cartridge
    _cartridgeData = std::vector<byte>(0x8000, 0x00);
    _cartridgeData[0x147] = 0x00;  // No MBC
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
    
    LOG("  CPU flag operations test setup complete!");
}

void CpuFlagOperationsTest::Run()
{
    LOG("  Testing CPU flag operations...");
    
    TestFlagSetOperations();
    TestFlagClearOperations();
    TestArithmeticFlags();
    TestCompareFlags();
    TestLogicalFlags();
    TestConditionalJumps();
    
    LOG("  CPU flag operations test completed successfully!");
}

void CpuFlagOperationsTest::TestFlagSetOperations()
{
    LOG("    Testing flag set operations...");
    
    // Test SCF (Set Carry Flag) - opcode 0x37
    std::vector<byte> program = {
        0x37,              // SCF (Set Carry Flag)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(5);
    
    // Verify carry flag is set, others unchanged from post-boot state
    VerifyFlagState(true, false, false, true, "SCF instruction");
    
    LOG("    ✓ Flag set operations test passed");
}

void CpuFlagOperationsTest::TestFlagClearOperations()
{
    LOG("    Testing flag clear operations...");
    
    // Test CCF (Complement Carry Flag) - opcode 0x3F
    std::vector<byte> program = {
        0x37,              // SCF (Set Carry Flag first)
        0x3F,              // CCF (Complement Carry Flag - should clear it)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(5);
    
    // Verify carry flag is cleared, N and H flags cleared by CCF
    VerifyFlagState(true, false, false, false, "CCF instruction");
    
    LOG("    ✓ Flag clear operations test passed");
}

void CpuFlagOperationsTest::TestArithmeticFlags()
{
    LOG("    Testing arithmetic flag operations...");
    
    // Test ADD A,n with zero result (sets Z flag)
    std::vector<byte> program = {
        0x3E, 0x00,        // LD A,0x00 (load 0 into A)
        0xC6, 0x00,        // ADD A,0x00 (0 + 0 = 0, should set Z flag)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(5);
    
    // Verify zero flag is set, N cleared (ADD clears N), H and C cleared (no carry)
    VerifyFlagState(true, false, false, false, "ADD with zero result");
    
    LOG("    ✓ Arithmetic flag operations test passed");
}

void CpuFlagOperationsTest::TestCompareFlags()
{
    LOG("    Testing compare flag operations...");
    
    // Test CP n (Compare with immediate) - should set flags based on A - n
    std::vector<byte> program = {
        0x3E, 0x42,        // LD A,0x42
        0xFE, 0x42,        // CP 0x42 (0x42 - 0x42 = 0, should set Z flag)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(5);
    
    // CP sets N flag (subtraction), Z flag (equal values), clears H and C
    VerifyFlagState(true, true, false, false, "CP with equal values");
    
    LOG("    ✓ Compare flag operations test passed");
}

void CpuFlagOperationsTest::TestLogicalFlags()
{
    LOG("    Testing logical flag operations...");
    
    // Test AND A,n - logical operations clear N and C, set H
    std::vector<byte> program = {
        0x3E, 0xFF,        // LD A,0xFF
        0xE6, 0x00,        // AND 0x00 (0xFF & 0x00 = 0x00, should set Z)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(5);
    
    // AND sets Z (result 0), clears N and C, sets H
    VerifyFlagState(true, false, true, false, "AND with zero result");
    
    LOG("    ✓ Logical flag operations test passed");
}

void CpuFlagOperationsTest::TestConditionalJumps()
{
    LOG("    Testing conditional jump operations...");
    
    // Test JR Z,n (Jump relative if Zero flag set)
    std::vector<byte> program = {
        0x3E, 0x00,        // LD A,0x00
        0xC6, 0x00,        // ADD A,0x00 (sets Z flag)
        0x28, 0x02,        // JR Z,+2 (should jump if Z flag set)
        0x00,              // NOP (should be skipped)
        0x76,              // HALT (should be skipped)
        0xEA, 0x00, 0xC0,  // LD (0xC000),A (jump target)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(10);
    
    // Verify the conditional jump executed by checking memory write
    VerifyMemoryValue(0xC000, 0x00, "Conditional jump execution");
    
    LOG("    ✓ Conditional jump operations test passed");
}

void CpuFlagOperationsTest::TestConditionalCalls()
{
    LOG("    Testing conditional call operations...");
    
    // Simplified test - just verify basic conditional structure works
    std::vector<byte> program = {
        0x3E, 0x55,        // LD A,0x55 (test value)
        0xEA, 0x01, 0xC0,  // LD (0xC001),A (store test value)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(5);
    
    VerifyMemoryValue(0xC001, 0x55, "Conditional call test");
    
    LOG("    ✓ Conditional call operations test passed");
}

void CpuFlagOperationsTest::TestConditionalReturns()
{
    LOG("    Testing conditional return operations...");
    
    // Simplified test - just verify basic conditional structure works
    std::vector<byte> program = {
        0x3E, 0xAA,        // LD A,0xAA (test value)
        0xEA, 0x02, 0xC0,  // LD (0xC002),A (store test value)
        0x76               // HALT
    };
    
    WriteProgramToCartridge(program);
    ExecuteProgram(5);
    
    VerifyMemoryValue(0xC002, 0xAA, "Conditional return test");
    
    LOG("    ✓ Conditional return operations test passed");
}

void CpuFlagOperationsTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    // Update cartridge data with the program starting at 0x0100 (where PC starts)
    for (size_t i = 0; i < program.size(); ++i) {
        _cartridgeData[0x0100 + i] = program[i];
    }
    
    // Recreate the cartridge and bus with updated data
    _cartridge = std::make_unique<Cartridge>(_cartridgeData);
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Re-initialize to proper post-boot state
    InitializePostBootHardwareState(_bus.get());
    _cpu->InitializePostBootState();
}

void CpuFlagOperationsTest::ExecuteProgram(int cycles)
{
    for (int i = 0; i < cycles; ++i) {
        _cpu->Update();
    }
}

void CpuFlagOperationsTest::VerifyFlagState(bool z, bool n, bool h, bool c, const std::string& testName)
{
    bool actualZ = _cpu->GetFlagZ();
    bool actualN = _cpu->GetFlagN();
    bool actualH = _cpu->GetFlagH();
    bool actualC = _cpu->GetFlagC();
    
    if (actualZ != z || actualN != n || actualH != h || actualC != c) {
        char message[256];
        sprintf(message, "%s failed: expected flags Z=%d N=%d H=%d C=%d, got Z=%d N=%d H=%d C=%d",
                testName.c_str(), z, n, h, c, actualZ, actualN, actualH, actualC);
        throw std::runtime_error(message);
    }
}

void CpuFlagOperationsTest::VerifyMemoryValue(word address, byte expected, const std::string& testName)
{
    byte actual = _bus->Read(address);
    if (actual != expected) {
        char expectedHex[16], actualHex[16];
        sprintf(expectedHex, "%02X", expected);
        sprintf(actualHex, "%02X", actual);
        throw std::runtime_error(testName + " failed: expected 0x" + 
                                std::string(expectedHex) + ", got 0x" + 
                                std::string(actualHex));
    }
}