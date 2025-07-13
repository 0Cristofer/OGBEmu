#include "ApuTest.h"
#include "Core/Logger.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>

ApuTest::ApuTest() : BaseTest("APU Test")
{
}

void ApuTest::Run()
{
    LOG("  Testing APU functionality...");
    
    TestInitialState();
    TestRegisterInitialization();
    TestEnableDisable();
    TestFrameSequencer();
    TestChannel1Registers();
    TestChannel2Registers();
    TestChannel3Registers();
    TestChannel4Registers();
    TestMasterControl();
    
    LOG("  APU test completed successfully!");
}

void ApuTest::Setup()
{
    LOG("  Setting up APU test...");
    
    _ioRegisters = std::make_unique<IoRegisters>();
    _apu = std::make_unique<Apu>(_ioRegisters.get());
    
    LOG("  APU test setup complete!");
}

void ApuTest::TestInitialState()
{
    LOG("    Testing initial state...");
    
    // APU should be disabled by default
    if (_apu->IsEnabled()) {
        throw std::runtime_error("APU should be disabled initially");
    }
    
    LOG("    Initial state test passed!");
}

void ApuTest::TestRegisterInitialization()
{
    LOG("    Testing register initialization...");
    
    // Test Channel 1 registers (NR10-NR14)
    AssertRegister(0xFF10, 0x80, "NR10 initial value");
    AssertRegister(0xFF11, 0xBF, "NR11 initial value");
    AssertRegister(0xFF12, 0xF3, "NR12 initial value");
    AssertRegister(0xFF13, 0x00, "NR13 initial value");
    AssertRegister(0xFF14, 0xBF, "NR14 initial value");
    
    // Test Channel 2 registers (NR20-NR24)
    AssertRegister(0xFF16, 0x3F, "NR21 initial value");
    AssertRegister(0xFF17, 0x00, "NR22 initial value");
    AssertRegister(0xFF18, 0x00, "NR23 initial value");
    AssertRegister(0xFF19, 0xBF, "NR24 initial value");
    
    // Test Channel 3 registers (NR30-NR34)
    AssertRegister(0xFF1A, 0x7F, "NR30 initial value");
    AssertRegister(0xFF1B, 0xFF, "NR31 initial value");
    AssertRegister(0xFF1C, 0x9F, "NR32 initial value");
    AssertRegister(0xFF1D, 0x00, "NR33 initial value");
    AssertRegister(0xFF1E, 0xBF, "NR34 initial value");
    
    // Test Channel 4 registers (NR40-NR44)
    AssertRegister(0xFF20, 0xFF, "NR41 initial value");
    AssertRegister(0xFF21, 0x00, "NR42 initial value");
    AssertRegister(0xFF22, 0x00, "NR43 initial value");
    AssertRegister(0xFF23, 0xBF, "NR44 initial value");
    
    // Test Control registers (NR50-NR52)
    AssertRegister(0xFF24, 0x77, "NR50 initial value");
    AssertRegister(0xFF25, 0xF3, "NR51 initial value");
    AssertRegister(0xFF26, 0xF1, "NR52 initial value");
    
    // Test Wave pattern RAM initialization
    for (word addr = 0xFF30; addr <= 0xFF3F; addr++) {
        AssertRegister(addr, 0x00, "Wave pattern RAM initial value");
    }
    
    LOG("    Register initialization test passed!");
}

void ApuTest::TestEnableDisable()
{
    LOG("    Testing APU enable/disable...");
    
    // Enable APU
    _apu->SetEnabled(true);
    if (!_apu->IsEnabled()) {
        throw std::runtime_error("APU should be enabled after SetEnabled(true)");
    }
    
    // Disable APU
    _apu->SetEnabled(false);
    if (_apu->IsEnabled()) {
        throw std::runtime_error("APU should be disabled after SetEnabled(false)");
    }
    
    // Re-enable for other tests
    _apu->SetEnabled(true);
    
    LOG("    APU enable/disable test passed!");
}

void ApuTest::TestFrameSequencer()
{
    LOG("    Testing frame sequencer...");
    
    // Enable APU
    _apu->SetEnabled(true);
    
    // The frame sequencer runs at 512 Hz (8192 CPU cycles per step)
    // We can't easily test the internal frame sequencer state,
    // but we can verify that Update() runs without errors
    
    SimulateCycles(8192);  // One frame sequencer step
    SimulateCycles(8192);  // Another step
    SimulateCycles(8192 * 6); // Complete a full frame sequencer cycle (8 steps)
    
    LOG("    Frame sequencer test passed!");
}

void ApuTest::TestChannel1Registers()
{
    LOG("    Testing Channel 1 register access...");
    
    // Test writing and reading NR10 (Sweep)
    WriteRegister(0xFF10, 0x55);
    AssertRegister(0xFF10, 0x55, "NR10 write/read");
    
    // Test writing and reading NR11 (Length/Duty)
    WriteRegister(0xFF11, 0xAA);
    AssertRegister(0xFF11, 0xAA, "NR11 write/read");
    
    // Test writing and reading NR12 (Volume)
    WriteRegister(0xFF12, 0x33);
    AssertRegister(0xFF12, 0x33, "NR12 write/read");
    
    // Test writing and reading NR13 (Frequency Low)
    WriteRegister(0xFF13, 0x77);
    AssertRegister(0xFF13, 0x77, "NR13 write/read");
    
    // Test writing and reading NR14 (Frequency High)
    WriteRegister(0xFF14, 0x99);
    AssertRegister(0xFF14, 0x99, "NR14 write/read");
    
    LOG("    Channel 1 register test passed!");
}

void ApuTest::TestChannel2Registers()
{
    LOG("    Testing Channel 2 register access...");
    
    // Test Channel 2 registers
    WriteRegister(0xFF16, 0x11);
    AssertRegister(0xFF16, 0x11, "NR21 write/read");
    
    WriteRegister(0xFF17, 0x22);
    AssertRegister(0xFF17, 0x22, "NR22 write/read");
    
    WriteRegister(0xFF18, 0x33);
    AssertRegister(0xFF18, 0x33, "NR23 write/read");
    
    WriteRegister(0xFF19, 0x44);
    AssertRegister(0xFF19, 0x44, "NR24 write/read");
    
    LOG("    Channel 2 register test passed!");
}

void ApuTest::TestChannel3Registers()
{
    LOG("    Testing Channel 3 register access...");
    
    // Test Channel 3 registers
    WriteRegister(0xFF1A, 0x55);
    AssertRegister(0xFF1A, 0x55, "NR30 write/read");
    
    WriteRegister(0xFF1B, 0x66);
    AssertRegister(0xFF1B, 0x66, "NR31 write/read");
    
    WriteRegister(0xFF1C, 0x77);
    AssertRegister(0xFF1C, 0x77, "NR32 write/read");
    
    WriteRegister(0xFF1D, 0x88);
    AssertRegister(0xFF1D, 0x88, "NR33 write/read");
    
    WriteRegister(0xFF1E, 0x99);
    AssertRegister(0xFF1E, 0x99, "NR34 write/read");
    
    // Test Wave pattern RAM
    for (word addr = 0xFF30; addr <= 0xFF3F; addr++) {
        byte testValue = static_cast<byte>(addr & 0xFF);
        WriteRegister(addr, testValue);
        AssertRegister(addr, testValue, "Wave pattern RAM write/read");
    }
    
    LOG("    Channel 3 register test passed!");
}

void ApuTest::TestChannel4Registers()
{
    LOG("    Testing Channel 4 register access...");
    
    // Test Channel 4 registers
    WriteRegister(0xFF20, 0xAA);
    AssertRegister(0xFF20, 0xAA, "NR41 write/read");
    
    WriteRegister(0xFF21, 0xBB);
    AssertRegister(0xFF21, 0xBB, "NR42 write/read");
    
    WriteRegister(0xFF22, 0xCC);
    AssertRegister(0xFF22, 0xCC, "NR43 write/read");
    
    WriteRegister(0xFF23, 0xDD);
    AssertRegister(0xFF23, 0xDD, "NR44 write/read");
    
    LOG("    Channel 4 register test passed!");
}

void ApuTest::TestMasterControl()
{
    LOG("    Testing master control registers...");
    
    // Test NR50 (Master Volume)
    WriteRegister(0xFF24, 0x12);
    AssertRegister(0xFF24, 0x12, "NR50 write/read");
    
    // Test NR51 (Sound Panning)
    WriteRegister(0xFF25, 0x34);
    AssertRegister(0xFF25, 0x34, "NR51 write/read");
    
    // Test NR52 (Sound Enable)
    WriteRegister(0xFF26, 0x56);
    AssertRegister(0xFF26, 0x56, "NR52 write/read");
    
    LOG("    Master control register test passed!");
}

void ApuTest::AssertRegister(word address, byte expected, const std::string& message)
{
    byte actual = _ioRegisters->Read(address);
    if (actual != expected)
    {
        std::ostringstream oss;
        oss << message << " - Expected: 0x" << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(expected) << ", Got: 0x" << static_cast<int>(actual);
        throw std::runtime_error(oss.str());
    }
}

void ApuTest::WriteRegister(word address, byte value)
{
    _ioRegisters->Write(address, value);
}

void ApuTest::SimulateCycles(int cycles)
{
    _apu->Update(cycles);
}