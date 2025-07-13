#include "Mbc3Test.h"
#include "Core/Logger.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>

Mbc3Test::Mbc3Test() : BaseTest("MBC3 Test")
{
}

void Mbc3Test::Setup()
{
    LOG("  Setting up MBC3 test...");
    
    // Create minimal boot ROM
    std::vector<byte> bootRomData(256, 0x00);
    
    // Create cartridge data with MBC3
    _cartridgeData.resize(0x10000, 0x00); // 64KB ROM (4 banks)
    
    // Fill ROM banks with test pattern for identification
    for (int bank = 0; bank < 4; bank++) {
        int bankStart = bank * 0x4000;
        for (int i = 0; i < 0x4000; i++) {
            _cartridgeData[bankStart + i] = static_cast<byte>((bank + 1) & 0xFF);
        }
    }
    
    // Set header bytes AFTER filling with test pattern
    _cartridgeData[0x0147] = 0x13; // MBC3+RAM+BATTERY cartridge type
    _cartridgeData[0x0148] = 0x01; // 64KB ROM size (4 banks) 
    _cartridgeData[0x0149] = 0x03; // 32KB RAM size (4 banks)
    
    // Create memory components
    _bootRom = std::make_unique<BootRom>(bootRomData);
    _cartridge = std::make_unique<Cartridge>(_cartridgeData);
    _vRam = std::make_unique<VRam>();
    _wRam = std::make_unique<WRam>();
    _wRamCgb = std::make_unique<WRamCgb>();
    _echoRam = std::make_unique<EchoRam>();
    _oam = std::make_unique<Oam>();
    _ioRegisters = std::make_unique<IoRegisters>();
    _hRam = std::make_unique<HRam>();
    
    // Create memory bus
    _bus = std::make_unique<Bus>(
        _bootRom.get(),
        _cartridge.get(),
        _vRam.get(),
        _wRam.get(),
        _wRamCgb.get(),
        _echoRam.get(),
        _oam.get(),
        _ioRegisters.get(),
        _hRam.get()
    );
    
    // Disable boot ROM by writing to 0xFF50
    _bus->Write(0xFF50, 0x01);
    
    LOG("  MBC3 test setup complete!");
}

void Mbc3Test::Run()
{
    LOG("  Testing MBC3 functionality...");
    
    TestBasicRomBankSwitching();
    TestRamBankSwitching();
    TestRamEnable();
    TestRtcRegisters();
    TestRtcLatchingMechanism();
    TestBoundaryConditions();
    
    LOG("  MBC3 test completed successfully!");
}

void Mbc3Test::TestBasicRomBankSwitching()
{
    LOG("    Testing basic ROM bank switching...");
    
    // Test bank 0 (fixed)
    AssertMemory(0x0001, 0x01, "Bank 0 read at 0x0001");
    
    // Test initial bank 1 at 0x4000 (MBC3 starts with bank 1 selected)
    AssertMemory(0x4000, 0x02, "Initial bank 1 read at 0x4000");
    
    // Switch to bank 2
    WriteMemory(0x2000, 0x02);
    AssertMemory(0x4000, 0x03, "Bank 2 read at 0x4000");
    
    // Switch to bank 3
    WriteMemory(0x2000, 0x03);
    AssertMemory(0x4000, 0x04, "Bank 3 read at 0x4000");
    AssertMemory(0x6000, 0x04, "Bank 3 read at 0x6000");
    
    LOG("    Basic ROM bank switching test passed!");
}

void Mbc3Test::TestRamBankSwitching()
{
    LOG("    Testing RAM bank switching...");
    
    // Enable RAM
    WriteMemory(0x0000, 0x0A);
    
    // Test RAM bank 0
    WriteMemory(0x4000, 0x00);  // Select RAM bank 0
    WriteMemory(0xA000, 0x42);  // Write to RAM
    AssertMemory(0xA000, 0x42, "RAM bank 0 write/read");
    
    // Test RAM bank 1
    WriteMemory(0x4000, 0x01);  // Select RAM bank 1
    WriteMemory(0xA000, 0x88);  // Write to RAM
    AssertMemory(0xA000, 0x88, "RAM bank 1 write/read");
    
    // Verify bank 0 is still isolated
    WriteMemory(0x4000, 0x00);  // Back to RAM bank 0
    AssertMemory(0xA000, 0x42, "RAM bank 0 isolation check");
    
    LOG("    RAM bank switching test passed!");
}

void Mbc3Test::TestRamEnable()
{
    LOG("    Testing RAM enable/disable...");
    
    // Disable RAM
    WriteMemory(0x0000, 0x00);
    WriteMemory(0x4000, 0x00);  // Select RAM bank 0
    byte disabledRead = ReadMemory(0xA000);
    if (disabledRead != 0xFF) {
        throw std::runtime_error("RAM disabled read should return 0xFF, got: " + std::to_string(disabledRead));
    }
    
    // Enable RAM
    WriteMemory(0x0000, 0x0A);
    WriteMemory(0xA000, 0x55);
    AssertMemory(0xA000, 0x55, "RAM enabled write/read");
    
    // Test partial enable (should still work)
    WriteMemory(0x0000, 0x1A);  // Upper bits set, lower 4 bits = 0x0A
    WriteMemory(0xA000, 0x77);
    AssertMemory(0xA000, 0x77, "RAM partial enable write/read");
    
    LOG("    RAM enable/disable test passed!");
}

void Mbc3Test::TestRtcRegisters()
{
    LOG("    Testing RTC registers...");
    
    // Enable RAM/RTC
    WriteMemory(0x0000, 0x0A);
    
    // Test RTC register access
    WriteMemory(0x4000, 0x08);  // Select RTC_S (seconds)
    WriteMemory(0xA000, 0x25);  // Write 37 seconds
    
    // Latch the RTC data to read it back
    WriteMemory(0x6000, 0x00);
    WriteMemory(0x6000, 0x01);
    
    AssertMemory(0xA000, 0x25, "RTC seconds register write/read");
    
    WriteMemory(0x4000, 0x09);  // Select RTC_M (minutes)
    WriteMemory(0xA000, 0x2D);  // Write 45 minutes (0x2D = 45 decimal)
    
    // Re-latch to read the updated minutes
    WriteMemory(0x6000, 0x00);
    WriteMemory(0x6000, 0x01);
    
    AssertMemory(0xA000, 0x2D, "RTC minutes register write/read");
    
    WriteMemory(0x4000, 0x0A);  // Select RTC_H (hours)
    WriteMemory(0xA000, 0x12);  // Write 18 hours
    
    // Re-latch to read the updated hours
    WriteMemory(0x6000, 0x00);
    WriteMemory(0x6000, 0x01);
    
    AssertMemory(0xA000, 0x12, "RTC hours register write/read");
    
    WriteMemory(0x4000, 0x0B);  // Select RTC_DL (day low)
    WriteMemory(0xA000, 0xFF);  // Write 255 to day low
    
    // Re-latch to read the updated day low
    WriteMemory(0x6000, 0x00);
    WriteMemory(0x6000, 0x01);
    
    AssertMemory(0xA000, 0xFF, "RTC day low register write/read");
    
    WriteMemory(0x4000, 0x0C);  // Select RTC_DH (day high + control)
    WriteMemory(0xA000, 0x81);  // Day high bit + day carry flag
    
    // Re-latch to read the updated day high
    WriteMemory(0x6000, 0x00);
    WriteMemory(0x6000, 0x01);
    
    AssertMemory(0xA000, 0x81, "RTC day high register write/read");
    
    LOG("    RTC registers test passed!");
}

void Mbc3Test::TestRtcLatchingMechanism()
{
    LOG("    Testing RTC latching mechanism...");
    
    // Enable RAM/RTC
    WriteMemory(0x0000, 0x0A);
    
    // Set up RTC values
    WriteMemory(0x4000, 0x08);  // Select RTC_S
    WriteMemory(0xA000, 0x30);  // 48 seconds
    
    WriteMemory(0x4000, 0x09);  // Select RTC_M
    WriteMemory(0xA000, 0x15);  // 21 minutes
    
    // Latch the clock data (0x00 -> 0x01 transition)
    WriteMemory(0x6000, 0x00);
    WriteMemory(0x6000, 0x01);
    
    // Verify latched values are accessible
    WriteMemory(0x4000, 0x08);  // Select RTC_S
    AssertMemory(0xA000, 0x30, "Latched seconds value");
    
    WriteMemory(0x4000, 0x09);  // Select RTC_M
    AssertMemory(0xA000, 0x15, "Latched minutes value");
    
    LOG("    RTC latching mechanism test passed!");
}

void Mbc3Test::TestBoundaryConditions()
{
    LOG("    Testing boundary conditions...");
    
    // Test maximum ROM bank (should wrap or be clamped)
    WriteMemory(0x2000, 0x7F);  // Maximum 7-bit value
    byte bankData = ReadMemory(0x4000);
    LOG("    Maximum ROM bank access - got data: " << std::hex << static_cast<int>(bankData));
    
    // Test bank 0 mapping (should map to bank 1)
    WriteMemory(0x2000, 0x00);
    AssertMemory(0x4000, 0x02, "Bank 0 maps to bank 1");
    
    // Test RTC register boundaries
    WriteMemory(0x0000, 0x0A);  // Enable RTC
    WriteMemory(0x4000, 0x07);  // Invalid RTC register (should not crash)
    WriteMemory(0xA000, 0x42);
    
    WriteMemory(0x4000, 0x0D);  // Invalid RTC register (should not crash)
    WriteMemory(0xA000, 0x42);
    
    LOG("    Boundary conditions test passed!");
}

void Mbc3Test::AssertMemory(word address, byte expected, const std::string& message)
{
    byte actual = ReadMemory(address);
    if (actual != expected)
    {
        std::ostringstream oss;
        oss << message << " - Expected: 0x" << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(expected) << ", Got: 0x" << static_cast<int>(actual);
        throw std::runtime_error(oss.str());
    }
    LOG("    ✓ " << message << " - expected 0x" << std::hex << static_cast<int>(expected) 
        << ", got 0x" << static_cast<int>(actual));
}

void Mbc3Test::WriteMemory(word address, byte value)
{
    _bus->Write(address, value);
}

byte Mbc3Test::ReadMemory(word address)
{
    return _bus->Read(address);
}