#include "PpuInterruptTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Core/Logger.h"
#include <cassert>

PpuInterruptTest::PpuInterruptTest() : BaseTest("PPU Interrupt Test")
{
}

void PpuInterruptTest::Setup()
{
    LOG("  Setting up PPU interrupt test...");
    
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
    
    // Create bus and components
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    
    _cpu = std::make_unique<TestCpu>(_bus.get());
    _screen = std::make_unique<MockScreen>();
    _ppu = std::make_unique<Ppu>(_bus.get(), _screen.get());
    
    // Initialize to proper post-boot state
    InitializePostBootHardwareState(_bus.get());
    _cpu->InitializePostBootState();
    
    // Disable boot ROM
    _bus->Write(AddressConstants::BootRomBank, 0x01);
    
    // Enable LCD
    _bus->Write(AddressConstants::LcdControl, 0x91); // LCD on, BG on
    
    LOG("  PPU interrupt test setup complete!");
}

void PpuInterruptTest::Run()
{
    LOG("  Testing PPU interrupt functionality...");
    
    TestStatRegisterModeTracking();
    TestPpuModeStateTransitions();
    TestVBlankInterruptTriggering();
    TestLyLycComparison();
    TestStatInterruptGeneration();
    TestHBlankInterrupt();
    TestOamInterrupt();
    TestLyLycInterrupt();
    
    LOG("  PPU interrupt test completed successfully!");
}

void PpuInterruptTest::TestVBlankInterruptTriggering()
{
    LOG("    Testing VBlank interrupt triggering...");
    
    // Clear interrupt flags
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Wait for VBlank entry (scanline 144)
    WaitForScanline(144);
    
    // Check that VBlank interrupt flag was set
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x01) == 0) {
        throw std::runtime_error("VBlank interrupt flag not set when entering VBlank");
    }
    
    LOG("    ✓ VBlank interrupt triggering test passed");
}

void PpuInterruptTest::TestStatRegisterModeTracking()
{
    LOG("    Testing STAT register mode tracking...");
    
    // Test VBlank mode (scanline 144+)
    WaitForScanline(144);
    byte statRegister = _bus->Read(AddressConstants::LcdStatus);
    byte mode = statRegister & 0x03;
    if (mode != 1) { // VBlank mode
        throw std::runtime_error("STAT register not showing VBlank mode during VBlank");
    }
    
    // Test visible scanline modes (wait for scanline 0)
    WaitForScanline(0);
    
    // Mode should start as OAM Search (mode 2) at beginning of scanline
    statRegister = _bus->Read(AddressConstants::LcdStatus);
    mode = statRegister & 0x03;
    if (mode != 2) { // OAM Search mode
        throw std::runtime_error("STAT register not showing OAM Search mode at start of scanline");
    }
    
    LOG("    ✓ STAT register mode tracking test passed");
}

void PpuInterruptTest::TestStatInterruptGeneration()
{
    LOG("    Testing STAT interrupt generation...");
    
    // Enable VBlank STAT interrupt (bit 4 of STAT)
    byte statRegister = _bus->Read(AddressConstants::LcdStatus);
    statRegister |= 0x10; // Enable VBlank STAT interrupt
    _bus->Write(AddressConstants::LcdStatus, statRegister);
    
    // Clear interrupt flags
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Wait for VBlank
    WaitForScanline(144);
    
    // Check that LCD/STAT interrupt flag was set
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x02) == 0) {
        throw std::runtime_error("STAT interrupt flag not set when VBlank STAT interrupt enabled");
    }
    
    LOG("    ✓ STAT interrupt generation test passed");
}

void PpuInterruptTest::TestLyLycComparison()
{
    LOG("    Testing LY=LYC comparison...");
    
    // Set LYC to match a specific scanline
    _bus->Write(AddressConstants::LcdYCompare, 10);
    
    // Wait for that scanline
    WaitForScanline(10);
    
    // Check that LY=LYC flag is set in STAT register
    byte statRegister = _bus->Read(AddressConstants::LcdStatus);
    if ((statRegister & 0x04) == 0) {
        throw std::runtime_error("LY=LYC flag not set when LY matches LYC");
    }
    
    // Test different scanline - flag should be clear
    WaitForScanline(11);
    statRegister = _bus->Read(AddressConstants::LcdStatus);
    if ((statRegister & 0x04) != 0) {
        throw std::runtime_error("LY=LYC flag set when LY does not match LYC");
    }
    
    LOG("    ✓ LY=LYC comparison test passed");
}

void PpuInterruptTest::TestPpuModeStateTransitions()
{
    LOG("    Testing PPU mode state transitions...");
    
    // Wait for start of visible scanline
    WaitForScanline(0);
    
    // Should start in OAM Search mode (2)
    byte statRegister = _bus->Read(AddressConstants::LcdStatus);
    byte mode = statRegister & 0x03;
    if (mode != 2) {
        throw std::runtime_error("Expected OAM Search mode at start of scanline");
    }
    
    // Execute cycles to transition through modes
    ExecuteCycles(80); // Should now be in Drawing mode (3)
    
    statRegister = _bus->Read(AddressConstants::LcdStatus);
    mode = statRegister & 0x03;
    if (mode != 3) {
        throw std::runtime_error("Expected Drawing mode after OAM Search cycles");
    }
    
    ExecuteCycles(172); // Should now be in HBlank mode (0)
    
    statRegister = _bus->Read(AddressConstants::LcdStatus);
    mode = statRegister & 0x03;
    if (mode != 0) {
        throw std::runtime_error("Expected HBlank mode after Drawing cycles");
    }
    
    LOG("    ✓ PPU mode state transitions test passed");
}

void PpuInterruptTest::TestHBlankInterrupt()
{
    LOG("    Testing HBlank interrupt...");
    
    // Enable HBlank STAT interrupt (bit 3 of STAT)
    EnableStatInterrupt(0x08);
    
    // Clear interrupt flags
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Wait for start of scanline and execute through to HBlank
    WaitForScanline(0);
    ExecuteCycles(80 + 172); // OAM Search + Drawing = HBlank
    
    // Check that LCD/STAT interrupt flag was set
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x02) == 0) {
        throw std::runtime_error("STAT interrupt flag not set for HBlank interrupt");
    }
    
    LOG("    ✓ HBlank interrupt test passed");
}

void PpuInterruptTest::TestOamInterrupt()
{
    LOG("    Testing OAM interrupt...");
    
    // Enable OAM STAT interrupt (bit 5 of STAT)
    EnableStatInterrupt(0x20);
    
    // Clear interrupt flags
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Wait for start of scanline (OAM Search mode)
    WaitForScanline(0);
    
    // Check that LCD/STAT interrupt flag was set
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x02) == 0) {
        throw std::runtime_error("STAT interrupt flag not set for OAM interrupt");
    }
    
    LOG("    ✓ OAM interrupt test passed");
}

void PpuInterruptTest::TestLyLycInterrupt()
{
    LOG("    Testing LY=LYC interrupt...");
    
    // Enable LY=LYC STAT interrupt (bit 6 of STAT)
    EnableStatInterrupt(0x40);
    
    // Set LYC to match a specific scanline
    _bus->Write(AddressConstants::LcdYCompare, 50);
    
    // Clear interrupt flags
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Wait for that scanline
    WaitForScanline(50);
    
    // Check that LCD/STAT interrupt flag was set
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x02) == 0) {
        throw std::runtime_error("STAT interrupt flag not set for LY=LYC interrupt");
    }
    
    LOG("    ✓ LY=LYC interrupt test passed");
}

void PpuInterruptTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    // Update cartridge data with the program starting at 0x0100
    for (size_t i = 0; i < program.size(); ++i) {
        _cartridgeData[0x0100 + i] = program[i];
    }
    
    // Recreate the cartridge
    _cartridge = std::make_unique<Cartridge>(_cartridgeData);
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    _cpu = std::make_unique<TestCpu>(_bus.get());
    _ppu = std::make_unique<Ppu>(_bus.get(), _screen.get());
    
    InitializePostBootHardwareState(_bus.get());
    _cpu->InitializePostBootState();
}

void PpuInterruptTest::ExecuteCycles(int cycles)
{
    for (int i = 0; i < cycles; i++) {
        _ppu->Update(1);
    }
}

void PpuInterruptTest::WaitForScanline(byte targetScanline)
{
    // Execute PPU cycles until we reach the target scanline
    byte currentScanline;
    int timeout = 100000; // Prevent infinite loops
    
    do {
        _ppu->Update(1);
        currentScanline = _bus->Read(AddressConstants::LcdY);
        timeout--;
        if (timeout <= 0) {
            throw std::runtime_error("Timeout waiting for scanline " + std::to_string(targetScanline));
        }
    } while (currentScanline != targetScanline);
}

void PpuInterruptTest::EnableStatInterrupt(byte interruptBits)
{
    byte statRegister = _bus->Read(AddressConstants::LcdStatus);
    statRegister |= interruptBits;
    _bus->Write(AddressConstants::LcdStatus, statRegister);
}