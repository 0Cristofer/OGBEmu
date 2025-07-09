#include "HRamTest.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/MBC/NoMbc.h"
#include "Emulator/GbConstants.h"
#include <stdexcept>

HRamTest::HRamTest() : BaseTest("HRam Test")
{
}

void HRamTest::Setup()
{
    LOG("  Setting up HRam test...");
    
    // Create test cartridge data (minimal ROM header)
    _cartridgeData.resize(0x8000, 0x00);
    _cartridgeData[0x0100] = 0x00; // NOP
    _cartridgeData[0x0101] = 0xC3; // JP 0x0150
    _cartridgeData[0x0102] = 0x50;
    _cartridgeData[0x0103] = 0x01;
    
    // Create boot ROM data
    _bootRomData.resize(0x100, 0x00);
    _bootRomData[0x00] = 0x31; // LD SP, 0xFFFE
    _bootRomData[0x01] = 0xFE;
    _bootRomData[0x02] = 0xFF;
    _bootRomData[0x03] = 0x76; // HALT
    
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
    
    // Create system components
    _bus = std::make_unique<Bus>(
        _bootRom.get(), _cartridge.get(), _vRam.get(), _wRam.get(), _wRamCgb.get(),
        _echoRam.get(), _oam.get(), _ioRegisters.get(), _hRam.get()
    );
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Initialize hardware to post-boot state
    InitializePostBootHardwareState(_bus.get());
    
    LOG("  HRam test setup complete!");
}

void HRamTest::Run()
{
    LOG("  Testing HRam functionality...");
    
    TestHRamBasicReadWrite();
    TestHRamAddressTranslation();
    TestHRamBoundaryConditions();
    TestHRamFastAccess();
    // TestHRamStackPointer();
    // TestHRamInterruptHandling();
    // TestHRamConstants();
    // LOG("  HRam test completed successfully!");
}

void HRamTest::TestHRamBasicReadWrite()
{
    LOG("    Testing basic HRam read/write...");
    
    // Test basic read/write operations
    _bus->Write(0xFF80, 0x42);
    VerifyHRamValue(0xFF80, 0x42, "Basic HRam write/read at 0xFF80");
    
    _bus->Write(0xFF81, 0x55);
    VerifyHRamValue(0xFF81, 0x55, "Basic HRam write/read at 0xFF81");
    
    // Test that previous value is preserved
    VerifyHRamValue(0xFF80, 0x42, "Previous HRam value preservation");
    
    // Test different values
    _bus->Write(0xFFC0, 0xAA);
    VerifyHRamValue(0xFFC0, 0xAA, "Basic HRam write/read at 0xFFC0");
    
    _bus->Write(0xFFFE, 0x33);
    VerifyHRamValue(0xFFFE, 0x33, "Basic HRam write/read at 0xFFFE");
    
    LOG("    ✓ Basic HRam read/write test passed");
}

void HRamTest::TestHRamAddressTranslation()
{
    LOG("    Testing HRam address translation...");
    
    // Test that HRam addresses are properly translated
    // HRam starts at 0xFF80, so 0xFF80 -> internal address 0x0000
    _bus->Write(0xFF80, 0x11);
    VerifyHRamValue(0xFF80, 0x11, "HRam address translation 0xFF80");
    
    // Test mid-range address
    _bus->Write(0xFFC0, 0x22);
    VerifyHRamValue(0xFFC0, 0x22, "HRam address translation 0xFFC0");
    
    // Test end address
    _bus->Write(0xFFFE, 0x33);
    VerifyHRamValue(0xFFFE, 0x33, "HRam address translation 0xFFFE");
    
    // Verify they don't interfere with each other
    VerifyHRamValue(0xFF80, 0x11, "HRam address isolation 0xFF80");
    VerifyHRamValue(0xFFC0, 0x22, "HRam address isolation 0xFFC0");
    
    LOG("    ✓ HRam address translation test passed");
}

void HRamTest::TestHRamBoundaryConditions()
{
    LOG("    Testing HRam boundary conditions...");
    
    // Test start boundary
    _bus->Write(0xFF80, 0x44);
    VerifyHRamValue(0xFF80, 0x44, "HRam start boundary 0xFF80");
    
    // Test end boundary
    _bus->Write(0xFFFE, 0x55);
    VerifyHRamValue(0xFFFE, 0x55, "HRam end boundary 0xFFFE");
    
    // Test just before start (should not affect HRam)
    _bus->Write(0xFF7F, 0x66);
    VerifyHRamValue(0xFF80, 0x44, "HRam isolation from 0xFF7F");
    
    // Test just after end (0xFFFF is IE register, not HRam)
    _bus->Write(0xFFFF, 0x77);
    VerifyHRamValue(0xFFFE, 0x55, "HRam isolation from 0xFFFF");
    
    LOG("    ✓ HRam boundary conditions test passed");
}

void HRamTest::TestHRamFastAccess()
{
    LOG("    Testing HRam fast access...");
    
    // HRam is designed for fast access - test rapid read/write operations
    const std::vector<byte> testData = {0x12, 0x34, 0x50, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    
    // Write test data rapidly
    word baseAddr = 0xFF80;
    for (size_t i = 0; i < testData.size(); i++)
    {
        _bus->Write(baseAddr + static_cast<word>(i), testData[i]);
    }
    
    // Read back and verify - use simple verification without complex logging
    for (size_t i = 0; i < testData.size(); i++)
    {
        byte readVal = _bus->Read(baseAddr + static_cast<word>(i));
        if (readVal != testData[i]) {
            throw std::runtime_error("HRam fast access test failed at offset " + std::to_string(i));
        }
    }
    
    LOG("    ✓ HRam fast access verification completed");
    
    // Test interleaved read/write
    _bus->Write(0xFF90, 0xAA);
    _bus->Write(0xFF91, 0xBB);
    
    byte interleaved1 = _bus->Read(0xFF90);
    byte interleaved2 = _bus->Read(0xFF91);
    
    if (interleaved1 != 0xAA || interleaved2 != 0xBB) {
        throw std::runtime_error("HRam interleaved read/write test failed");
    }
    
    LOG("    ✓ HRam interleaved read/write operations completed");
}

void HRamTest::TestHRamStackPointer()
{
    // Test HRam usage for stack pointer storage
    // Game Boy stack pointer is typically initialized to 0xFFFE
    
    // Store stack pointer value in HRam
    _bus->Write(0xFF80, 0xFE); // SP low byte
    _bus->Write(0xFF81, 0xFF); // SP high byte
    
    // Test stack pointer manipulation
    _bus->Write(0xFF82, 0xFC); // Decremented SP low byte
    _bus->Write(0xFF83, 0xFF); // SP high byte unchanged
    
    // Verify the values were stored correctly
    byte sp_low = _bus->Read(0xFF80);
    byte sp_high = _bus->Read(0xFF81);
    byte sp_dec_low = _bus->Read(0xFF82);
    byte sp_dec_high = _bus->Read(0xFF83);
    
    if (sp_low != 0xFE || sp_high != 0xFF || sp_dec_low != 0xFC || sp_dec_high != 0xFF) {
        throw std::runtime_error("HRam stack pointer test failed");
    }
}

void HRamTest::TestHRamInterruptHandling()
{
    // Test HRam usage for interrupt-related data
    // Store interrupt vectors and flags
    
    // Store interrupt vectors
    _bus->Write(0xFF88, 0x40); // VBlank vector low
    _bus->Write(0xFF89, 0x00); // VBlank vector high
    _bus->Write(0xFF8A, 0x48); // LCD STAT vector low
    _bus->Write(0xFF8B, 0x00); // LCD STAT vector high
    
    // Store interrupt state
    _bus->Write(0xFF8C, 0x01); // Interrupt enabled flag
    _bus->Write(0xFF8D, 0x00); // Interrupt disabled flag
    
    // Test critical section data
    _bus->Write(0xFF8E, 0x42); // Critical section data
    
    // Verify the values were stored correctly
    byte vblank_low = _bus->Read(0xFF88);
    byte vblank_high = _bus->Read(0xFF89);
    byte lcd_stat_low = _bus->Read(0xFF8A);
    byte lcd_stat_high = _bus->Read(0xFF8B);
    byte int_enabled = _bus->Read(0xFF8C);
    byte int_disabled = _bus->Read(0xFF8D);
    byte critical_data = _bus->Read(0xFF8E);
    
    if (vblank_low != 0x40 || vblank_high != 0x00 || lcd_stat_low != 0x48 || lcd_stat_high != 0x00 ||
        int_enabled != 0x01 || int_disabled != 0x00 || critical_data != 0x42) {
        throw std::runtime_error("HRam interrupt handling test failed");
    }
}

void HRamTest::TestHRamConstants()
{
    // Test that HRam constants are correct
    if (AddressConstants::StartHRamAddress != 0xFF80)
    {
        throw std::runtime_error("HRam StartHRamAddress constant incorrect");
    }
    if (AddressConstants::EndHRamAddress != 0xFFFE)
    {
        throw std::runtime_error("HRam EndHRamAddress constant incorrect");
    }
    
    // Test HRam size calculation
    word expectedSize = AddressConstants::EndHRamAddress - AddressConstants::StartHRamAddress + 1;
    if (expectedSize != 0x7F) // 127 bytes
    {
        throw std::runtime_error("HRam size calculation incorrect");
    }
    
    // Test that 0xFFFF is not part of HRam (it's IE register)
    if (AddressConstants::EndHRamAddress == 0xFFFF)
    {
        throw std::runtime_error("HRam should not include 0xFFFF (IE register)");
    }
}

void HRamTest::VerifyHRamValue(word address, byte expected, const std::string& testName)
{
    byte actual = _bus->Read(address);
    if (actual == expected) {
        std::string expectedStr = std::to_string(static_cast<int>(expected));
        std::string actualStr = std::to_string(static_cast<int>(actual));
        LOG("    ✓ " + testName + " - expected 0x" + expectedStr + ", got 0x" + actualStr);
    } else {
        std::string expectedStr = std::to_string(static_cast<int>(expected));
        std::string actualStr = std::to_string(static_cast<int>(actual));
        LOG("    ✗ " + testName + " - expected 0x" + expectedStr + ", got 0x" + actualStr);
        throw std::runtime_error(testName + " test failed");
    }
}

void HRamTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size() && i < _cartridgeData.size(); i++)
    {
        _cartridgeData[i] = program[i];
    }
}

void HRamTest::ExecuteInstructions(int count)
{
    for (int i = 0; i < count; i++)
    {
        _cpu->Update();
    }
}