#include "PpuTest.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/MBC/NoMbc.h"
#include "Emulator/GbConstants.h"
#include <stdexcept>

PpuTest::PpuTest() : BaseTest("PPU Test")
{
}

void PpuTest::Setup()
{
    LOG("  Setting up PPU test...");
    
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
    _screen = std::make_unique<MockScreen>();
    _ppu = std::make_unique<Ppu>(_vRam.get(), _oam.get(), _ioRegisters.get(), _screen.get());
    
    // Initialize hardware to post-boot state
    InitializePostBootHardwareState(_bus.get());
    
    LOG("  PPU test setup complete!");
}

void PpuTest::Run()
{
    LOG("  Testing PPU functionality...");
    
    TestPpuInitialization();
    TestLcdDisabled();
    TestScanlineTimingBasic();
    TestScanlineOverflow();
    TestVBlankTiming();
    TestLcdYRegisterUpdate();
    TestPpuConstants();
    
    LOG("  PPU test completed successfully!");
}

void PpuTest::TestPpuInitialization()
{
    LOG("    Testing PPU initialization...");
    
    // PPU should be properly initialized
    if (_ppu == nullptr) {
        throw std::runtime_error("PPU initialization failed");
    }
    
    // Initial LCD Y should be 0x90 (post-boot state)
    VerifyLcdY(0x90, "Initial LCD Y register");
    
    LOG("    ✓ PPU initialization test passed");
}

void PpuTest::TestLcdDisabled()
{
    LOG("    Testing LCD disabled state...");
    
    // Disable LCD by clearing bit 7 of LCDC
    _ioRegisters->Write(AddressConstants::LcdControl, 0x00);
    
    // Update PPU with some cycles
    _ppu->Update(456); // One full scanline worth of cycles
    
    // When LCD is disabled, LY should be 0 and scanline should reset
    VerifyLcdY(0x00, "LCD disabled LY register");
    
    // Re-enable LCD
    _ioRegisters->Write(AddressConstants::LcdControl, 0x91);
    
    LOG("    ✓ LCD disabled state test passed");
}

void PpuTest::TestScanlineTimingBasic()
{
    LOG("    Testing basic scanline timing...");
    
    // Start from a known state
    _ioRegisters->Write(AddressConstants::LcdY, 0x00);
    
    // Update PPU with exactly one scanline worth of cycles
    _ppu->Update(456);
    
    // LY should advance by 1
    VerifyLcdY(0x01, "Basic scanline timing");
    
    // Update with another scanline
    _ppu->Update(456);
    VerifyLcdY(0x02, "Second scanline timing");
    
    LOG("    ✓ Basic scanline timing test passed");
}

void PpuTest::TestScanlineOverflow()
{
    LOG("    Testing scanline overflow...");
    
    // Set LY to near maximum (153 is the last scanline)
    _ioRegisters->Write(AddressConstants::LcdY, 153);
    
    // Update PPU with one scanline worth of cycles
    _ppu->Update(456);
    
    // Should wrap around to 0
    VerifyLcdY(0x00, "Scanline overflow to 0");
    
    LOG("    ✓ Scanline overflow test passed");
}

void PpuTest::TestVBlankTiming()
{
    LOG("    Testing VBlank timing...");
    
    // Set LY to scanline 144 (start of VBlank)
    _ioRegisters->Write(AddressConstants::LcdY, 144);
    
    // Update PPU with one scanline worth of cycles
    // Note: This may trigger RenderFrame() but since screen is null, it should be safe
    _ppu->Update(456);
    
    // Should be in VBlank period (scanline 145)
    VerifyLcdY(145, "VBlank timing");
    
    // Test end of VBlank (scanline 153 -> 0)
    _ioRegisters->Write(AddressConstants::LcdY, 153);
    _ppu->Update(456);
    VerifyLcdY(0x00, "VBlank end timing");
    
    LOG("    ✓ VBlank timing test passed");
}

void PpuTest::TestLcdYRegisterUpdate()
{
    LOG("    Testing LCD Y register updates...");
    
    // Test that LY register is properly updated during PPU cycles
    _ioRegisters->Write(AddressConstants::LcdY, 0x00);
    
    // Update with partial scanline cycles
    _ppu->Update(200); // Less than a full scanline
    
    // LY should still be 0
    VerifyLcdY(0x00, "Partial scanline LY");
    
    // Complete the scanline
    _ppu->Update(256); // 200 + 256 = 456 total
    
    // Now LY should be 1
    VerifyLcdY(0x01, "Complete scanline LY");
    
    LOG("    ✓ LCD Y register update test passed");
}

void PpuTest::TestPpuConstants()
{
    LOG("    Testing PPU constants...");
    
    // Test that PPU constants are correct
    if (Ppu::CYCLES_PER_SCANLINE != 456) {
        throw std::runtime_error("PPU CYCLES_PER_SCANLINE constant incorrect");
    }
    if (Ppu::VISIBLE_SCANLINES != 144) {
        throw std::runtime_error("PPU VISIBLE_SCANLINES constant incorrect");
    }
    if (Ppu::VBLANK_SCANLINES != 10) {
        throw std::runtime_error("PPU VBLANK_SCANLINES constant incorrect");
    }
    if (Ppu::TOTAL_SCANLINES != 154) {
        throw std::runtime_error("PPU TOTAL_SCANLINES constant incorrect");
    }
    
    LOG("    ✓ PPU constants test passed");
}

void PpuTest::VerifyLcdY(byte expected, const std::string& testName)
{
    byte actual = _ioRegisters->Read(AddressConstants::LcdY);
    if (actual == expected) {
        LOG("    ✓ " + testName + " - expected 0x" + std::to_string(expected) + ", got 0x" + std::to_string(actual));
    } else {
        LOG("    ✗ " + testName + " - expected 0x" + std::to_string(expected) + ", got 0x" + std::to_string(actual));
        throw std::runtime_error(testName + " test failed");
    }
}

void PpuTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size() && i < _cartridgeData.size(); i++)
    {
        _cartridgeData[i] = program[i];
    }
}

void PpuTest::ExecuteInstructions(int count)
{
    for (int i = 0; i < count; i++)
    {
        _cpu->Update();
    }
}