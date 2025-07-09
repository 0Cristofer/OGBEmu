#include "IoRegistersTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Core/Logger.h"
#include <cassert>

IoRegistersTest::IoRegistersTest() : BaseTest("IO Registers Test")
{
}

void IoRegistersTest::Setup()
{
    LOG("  Setting up IO registers test...");
    
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
    
    // Create bus
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    
    // Create CPU
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Initialize to proper post-boot state
    InitializePostBootHardwareState(_bus.get());
    
    LOG("  IO registers test setup complete!");
}

void IoRegistersTest::Run()
{
    LOG("  Testing IO registers...");
    
    TestBasicIoRegisterAccess();
    TestIoRegisterAddressBoundaries();
    TestLcdControlRegister();
    TestLcdStatusRegister();
    TestScrollRegisters();
    TestLcdYRegister();
    TestPaletteRegisters();
    TestWindowRegisters();
    TestDmaRegister();
    TestBootRomBankRegister();
    TestInterruptFlagRegister();
    TestSpecialBehaviorRegisters();
    
    LOG("  IO registers test completed successfully!");
}

void IoRegistersTest::TestBasicIoRegisterAccess()
{
    LOG("    Testing basic IO register access...");
    
    // Test basic read/write functionality for IO registers
    // Start with a simple register that should be writable
    word testAddress = 0xFF00;  // Joypad register
    
    // Write a test value
    _bus->Write(testAddress, 0x42);
    byte readValue = _bus->Read(testAddress);
    
    if (readValue != 0x42) {
        throw std::runtime_error("Basic IO register write/read failed: expected 0x42, got 0x" + 
                                std::to_string(readValue));
    }
    
    // Test another register
    testAddress = 0xFF01;  // Serial data register
    _bus->Write(testAddress, 0x55);
    readValue = _bus->Read(testAddress);
    
    if (readValue != 0x55) {
        throw std::runtime_error("Basic IO register write/read failed: expected 0x55, got 0x" + 
                                std::to_string(readValue));
    }
    
    LOG("    ✓ Basic IO register access test passed");
}

void IoRegistersTest::TestIoRegisterAddressBoundaries()
{
    LOG("    Testing IO register address boundaries...");
    
    // Test start boundary (0xFF00)
    _bus->Write(AddressConstants::StartIoRegistersAddress, 0xAA);
    VerifyRegisterValue(AddressConstants::StartIoRegistersAddress, 0xAA, "IO registers start boundary");
    
    // Test end boundary (0xFF7F)
    _bus->Write(AddressConstants::EndIoRegistersAddress, 0xBB);
    VerifyRegisterValue(AddressConstants::EndIoRegistersAddress, 0xBB, "IO registers end boundary");
    
    // Test middle address
    word middleAddress = 0xFF30;  // Wave pattern RAM
    _bus->Write(middleAddress, 0xCC);
    VerifyRegisterValue(middleAddress, 0xCC, "IO registers middle address");
    
    LOG("    ✓ IO register address boundaries test passed");
}

void IoRegistersTest::TestLcdControlRegister()
{
    LOG("    Testing LCD Control register...");
    
    // Test LCDC register (0xFF40)
    word lcdcAddress = AddressConstants::LcdControl;
    
    // Test writing various LCD control values
    _bus->Write(lcdcAddress, 0x91);  // LCD on, BG on, sprites off (common boot value)
    VerifyRegisterValue(lcdcAddress, 0x91, "LCDC register boot value");
    
    _bus->Write(lcdcAddress, 0x80);  // LCD on, everything else off
    VerifyRegisterValue(lcdcAddress, 0x80, "LCDC register LCD on only");
    
    _bus->Write(lcdcAddress, 0x00);  // LCD off
    VerifyRegisterValue(lcdcAddress, 0x00, "LCDC register LCD off");
    
    _bus->Write(lcdcAddress, 0xFF);  // All features on
    VerifyRegisterValue(lcdcAddress, 0xFF, "LCDC register all features on");
    
    LOG("    ✓ LCD Control register test passed");
}

void IoRegistersTest::TestLcdStatusRegister()
{
    LOG("    Testing LCD Status register...");
    
    // Test STAT register (0xFF41)
    word statAddress = AddressConstants::LcdStatus;
    
    // Test writing to STAT register
    _bus->Write(statAddress, 0x40);  // LYC=LY interrupt enable
    VerifyRegisterValue(statAddress, 0x40, "STAT register LYC interrupt");
    
    _bus->Write(statAddress, 0x20);  // Mode 2 OAM interrupt enable
    VerifyRegisterValue(statAddress, 0x20, "STAT register OAM interrupt");
    
    _bus->Write(statAddress, 0x10);  // Mode 1 VBlank interrupt enable
    VerifyRegisterValue(statAddress, 0x10, "STAT register VBlank interrupt");
    
    _bus->Write(statAddress, 0x08);  // Mode 0 HBlank interrupt enable
    VerifyRegisterValue(statAddress, 0x08, "STAT register HBlank interrupt");
    
    LOG("    ✓ LCD Status register test passed");
}

void IoRegistersTest::TestScrollRegisters()
{
    LOG("    Testing scroll registers...");
    
    // Test SCY register (0xFF42)
    _bus->Write(AddressConstants::ScrollY, 0x10);
    VerifyRegisterValue(AddressConstants::ScrollY, 0x10, "SCY register");
    
    // Test SCX register (0xFF43)
    _bus->Write(AddressConstants::ScrollX, 0x20);
    VerifyRegisterValue(AddressConstants::ScrollX, 0x20, "SCX register");
    
    // Test boundary values
    _bus->Write(AddressConstants::ScrollY, 0x00);
    VerifyRegisterValue(AddressConstants::ScrollY, 0x00, "SCY register minimum");
    
    _bus->Write(AddressConstants::ScrollX, 0xFF);
    VerifyRegisterValue(AddressConstants::ScrollX, 0xFF, "SCX register maximum");
    
    LOG("    ✓ Scroll registers test passed");
}

void IoRegistersTest::TestLcdYRegister()
{
    LOG("    Testing LCD Y register...");
    
    // Test LY register (0xFF44) - usually read-only but we can test basic access
    word lyAddress = AddressConstants::LcdY;
    
    // LY register is typically read-only and controlled by PPU
    // We'll test that we can read from it (should return 0 initially)
    byte lyValue = _bus->Read(lyAddress);
    
    // LY should be readable (exact value depends on PPU state)
    // Just verify we can read without crashing
    LOG("    LY register value: 0x" << std::hex << (int)lyValue);
    
    // Test LYC register (0xFF45) - this should be writable
    _bus->Write(AddressConstants::LcdYCompare, 0x90);
    VerifyRegisterValue(AddressConstants::LcdYCompare, 0x90, "LYC register");
    
    _bus->Write(AddressConstants::LcdYCompare, 0x00);
    VerifyRegisterValue(AddressConstants::LcdYCompare, 0x00, "LYC register zero");
    
    LOG("    ✓ LCD Y register test passed");
}

void IoRegistersTest::TestPaletteRegisters()
{
    LOG("    Testing palette registers...");
    
    // Test BGP register (0xFF47)
    _bus->Write(AddressConstants::BackgroundPalette, 0xE4);  // Common palette value
    VerifyRegisterValue(AddressConstants::BackgroundPalette, 0xE4, "BGP register");
    
    // Test OBP0 register (0xFF48)
    _bus->Write(AddressConstants::ObjectPalette0, 0xE0);
    VerifyRegisterValue(AddressConstants::ObjectPalette0, 0xE0, "OBP0 register");
    
    // Test OBP1 register (0xFF49)
    _bus->Write(AddressConstants::ObjectPalette1, 0xE1);
    VerifyRegisterValue(AddressConstants::ObjectPalette1, 0xE1, "OBP1 register");
    
    // Test various palette combinations
    _bus->Write(AddressConstants::BackgroundPalette, 0x1B);  // Light to dark gradient
    VerifyRegisterValue(AddressConstants::BackgroundPalette, 0x1B, "BGP register gradient");
    
    LOG("    ✓ Palette registers test passed");
}

void IoRegistersTest::TestWindowRegisters()
{
    LOG("    Testing window registers...");
    
    // Test WY register (0xFF4A)
    _bus->Write(AddressConstants::WindowY, 0x50);
    VerifyRegisterValue(AddressConstants::WindowY, 0x50, "WY register");
    
    // Test WX register (0xFF4B)
    _bus->Write(AddressConstants::WindowX, 0x60);
    VerifyRegisterValue(AddressConstants::WindowX, 0x60, "WX register");
    
    // Test boundary values
    _bus->Write(AddressConstants::WindowY, 0x00);
    VerifyRegisterValue(AddressConstants::WindowY, 0x00, "WY register minimum");
    
    _bus->Write(AddressConstants::WindowX, 0xFF);
    VerifyRegisterValue(AddressConstants::WindowX, 0xFF, "WX register maximum");
    
    LOG("    ✓ Window registers test passed");
}

void IoRegistersTest::TestDmaRegister()
{
    LOG("    Testing DMA register...");
    
    // Test DMA register (0xFF46)
    word dmaAddress = AddressConstants::DmaStart;
    
    // DMA register is write-only and triggers DMA transfer
    // We'll test that we can write to it without crashing
    _bus->Write(dmaAddress, 0xC0);  // DMA from 0xC000 (WRAM)
    
    // Reading from DMA register behavior depends on implementation
    // Just verify we can access it
    byte dmaValue = _bus->Read(dmaAddress);
    LOG("    DMA register read value: 0x" << std::hex << (int)dmaValue);
    
    LOG("    ✓ DMA register test passed");
}

void IoRegistersTest::TestBootRomBankRegister()
{
    LOG("    Testing Boot ROM bank register...");
    
    // Test boot ROM bank register (0xFF50)
    word bootRomAddress = AddressConstants::BootRomBank;
    
    // This register is write-once to disable boot ROM
    // Test writing to it
    _bus->Write(bootRomAddress, 0x01);
    
    // Reading behavior depends on implementation
    byte bootRomValue = _bus->Read(bootRomAddress);
    LOG("    Boot ROM bank register value: 0x" << std::hex << (int)bootRomValue);
    
    LOG("    ✓ Boot ROM bank register test passed");
}

void IoRegistersTest::TestInterruptFlagRegister()
{
    LOG("    Testing Interrupt Flag register...");
    
    // Test IF register (0xFF0F)
    word ifAddress = AddressConstants::InterruptFlag;
    
    // Test writing interrupt flags
    _bus->Write(ifAddress, 0x01);  // VBlank interrupt
    VerifyRegisterValue(ifAddress, 0x01, "IF register VBlank");
    
    _bus->Write(ifAddress, 0x02);  // LCD interrupt
    VerifyRegisterValue(ifAddress, 0x02, "IF register LCD");
    
    _bus->Write(ifAddress, 0x1F);  // All interrupts
    VerifyRegisterValue(ifAddress, 0x1F, "IF register all interrupts");
    
    _bus->Write(ifAddress, 0x00);  // Clear all
    VerifyRegisterValue(ifAddress, 0x00, "IF register clear");
    
    LOG("    ✓ Interrupt Flag register test passed");
}

void IoRegistersTest::TestSpecialBehaviorRegisters()
{
    LOG("    Testing special behavior registers...");
    
    // Test some registers with special behavior
    // Timer registers (0xFF04-0xFF07)
    word divRegister = 0xFF04;  // DIV register
    word timaRegister = 0xFF05; // TIMA register
    word tmaRegister = 0xFF06;  // TMA register
    word tacRegister = 0xFF07;  // TAC register
    
    // Test timer registers (these might have special behavior)
    _bus->Write(timaRegister, 0x80);
    VerifyRegisterValue(timaRegister, 0x80, "TIMA register");
    
    _bus->Write(tmaRegister, 0x90);
    VerifyRegisterValue(tmaRegister, 0x90, "TMA register");
    
    _bus->Write(tacRegister, 0x07);
    VerifyRegisterValue(tacRegister, 0x07, "TAC register");
    
    // Test serial registers (0xFF01-0xFF02)
    word sbRegister = 0xFF01;   // SB register
    word scRegister = 0xFF02;   // SC register
    
    _bus->Write(sbRegister, 0xAB);
    VerifyRegisterValue(sbRegister, 0xAB, "SB register");
    
    _bus->Write(scRegister, 0x81);
    VerifyRegisterValue(scRegister, 0x81, "SC register");
    
    LOG("    ✓ Special behavior registers test passed");
}

void IoRegistersTest::VerifyRegisterValue(word address, byte expected, const std::string& testName)
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

void IoRegistersTest::VerifyRegisterDefault(word address, byte expected, const std::string& registerName)
{
    byte actual = _bus->Read(address);
    if (actual != expected) {
        char expectedHex[16], actualHex[16];
        sprintf(expectedHex, "%02X", expected);
        sprintf(actualHex, "%02X", actual);
        throw std::runtime_error(registerName + " default value failed: expected 0x" + 
                                std::string(expectedHex) + ", got 0x" + 
                                std::string(actualHex));
    }
}

void IoRegistersTest::WriteProgramToCartridge(const std::vector<byte>& program)
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

void IoRegistersTest::ExecuteInstructions(int count)
{
    for (int i = 0; i < count; i++) {
        _cpu->Update();
    }
}