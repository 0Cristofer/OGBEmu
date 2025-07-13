#include "Mbc5Test.h"
#include "Core/Logger.h"
#include "Emulator/GbConstants.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>

Mbc5Test::Mbc5Test() : BaseTest("MBC5 Test")
{
}

void Mbc5Test::Run()
{
    LOG("  Testing MBC5 functionality...");
    
    TestBasicRomBankSwitching();
    TestExtended9BitRomBanking();
    TestRamBankSwitching();
    TestRamEnable();
    TestBankBoundaries();
    TestInvalidBankAccess();
    
    LOG("  MBC5 test completed successfully!");
}

void Mbc5Test::Setup()
{
    LOG("  Setting up MBC5 test...");
    
    // Create minimal boot ROM
    std::vector<byte> bootRomData(256, 0x00);
    
    // Create MBC5 ROM with 32 banks (512KB) and 16 RAM banks
    _cartridgeData = CreateMbc5Rom(32, GbConstants::RamSizeFlag16Bank);
    
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
    
    // Create bus with all components
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), _wRam.get(),
                                 _wRamCgb.get(), _echoRam.get(), _oam.get(), _ioRegisters.get(),
                                 _hRam.get());
    
    // Create CPU
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Initialize post-boot state
    _cpu->SetPC(0x0100);
    _cpu->SetSP(0xFFFE);
    _bus->Write(AddressConstants::BootRomBank, 0x01); // Disable boot ROM
    
    LOG("  MBC5 test setup complete!");
}

void Mbc5Test::TestBasicRomBankSwitching()
{
    LOG("    Testing basic ROM bank switching...");
    
    // Test bank 0 (fixed bank at 0x0000-0x3FFF)
    VerifyMemoryValue(0x0000, 0x00, "Bank 0 read at start");
    VerifyMemoryValue(0x3FFF, 0x00, "Bank 0 read at end");
    
    // Test default bank 1 (0x4000-0x7FFF)
    VerifyMemoryValue(0x4000, 0x01, "Default bank 1 at 0x4000");
    
    // Switch to bank 2 (write to low 8 bits register)
    _bus->Write(0x2000, 0x02);
    VerifyMemoryValue(0x4000, 0x02, "Bank 2 read at 0x4000");
    
    // Switch to bank 5
    _bus->Write(0x2000, 0x05);
    VerifyMemoryValue(0x4000, 0x05, "Bank 5 read at 0x4000");
    
    // Switch back to bank 1
    _bus->Write(0x2000, 0x01);
    VerifyMemoryValue(0x4000, 0x01, "Bank 1 read at 0x4000");
    
    LOG("    Basic ROM bank switching test passed!");
}

void Mbc5Test::TestExtended9BitRomBanking()
{
    LOG("    Testing 9-bit ROM banking...");
    
    // Test switching to bank 256 (requires 9th bit)
    // Set low 8 bits to 0x00
    _bus->Write(0x2000, 0x00);
    // Set high bit to 1 (bit 0 of 0x3000 register)
    _bus->Write(0x3000, 0x01);
    // This should result in bank 256 (0x100) - wraps to bank 0 in our 32-bank ROM
    VerifyMemoryValue(0x4000, 0x00, "Bank 256 read (wraps to bank 0 in our 32-bank ROM)");
    
    // Test bank 257
    _bus->Write(0x2000, 0x01);
    _bus->Write(0x3000, 0x01);
    // This should result in bank 257 (0x101) - wraps to bank 1 in our 32-bank ROM
    VerifyMemoryValue(0x4000, 0x01, "Bank 257 read (wraps to bank 1 in our 32-bank ROM)");
    
    // Reset to normal banking
    _bus->Write(0x3000, 0x00);
    _bus->Write(0x2000, 0x01);
    VerifyMemoryValue(0x4000, 0x01, "Normal bank 1 after reset");
    
    LOG("    9-bit ROM banking test passed!");
}

void Mbc5Test::TestRamBankSwitching()
{
    LOG("    Testing RAM bank switching...");
    
    // Enable RAM first
    _bus->Write(0x0000, 0x0A);
    
    // Test default RAM bank 0
    _bus->Write(0xA000, 0xAB);
    VerifyMemoryValue(0xA000, 0xAB, "Read from RAM bank 0");
    
    // Switch to RAM bank 1
    _bus->Write(0x4000, 0x01);
    VerifyMemoryValue(0xA000, 0x00, "RAM bank 1 should be empty");
    _bus->Write(0xA000, 0xCD);
    VerifyMemoryValue(0xA000, 0xCD, "Read from RAM bank 1");
    
    // Switch back to RAM bank 0
    _bus->Write(0x4000, 0x00);
    VerifyMemoryValue(0xA000, 0xAB, "RAM bank 0 should retain data");
    
    // Test RAM bank 2
    _bus->Write(0x4000, 0x02);
    VerifyMemoryValue(0xA000, 0x00, "RAM bank 2 should be empty");
    
    LOG("    RAM bank switching test passed!");
}

void Mbc5Test::TestRamEnable()
{
    LOG("    Testing RAM enable/disable...");
    
    // Test RAM disabled by default (should read 0xFF)
    _bus->Write(0x0000, 0x00);
    VerifyMemoryValue(0xA000, 0xFF, "RAM disabled should read 0xFF");
    
    // Test enabling RAM
    _bus->Write(0x0000, 0x0A);
    _bus->Write(0xA000, 0x42);
    VerifyMemoryValue(0xA000, 0x42, "Read from enabled RAM");
    
    // Test disabling RAM again
    _bus->Write(0x0000, 0x00);
    VerifyMemoryValue(0xA000, 0xFF, "RAM disabled should read 0xFF again");
    
    // Test that writes to disabled RAM are ignored
    _bus->Write(0xA000, 0x99);
    _bus->Write(0x0000, 0x0A);
    VerifyMemoryValue(0xA000, 0x42, "Previous data should be preserved");
    
    LOG("    RAM enable/disable test passed!");
}

void Mbc5Test::TestBankBoundaries()
{
    LOG("    Testing bank boundaries...");
    
    // Test boundary between fixed bank 0 and switchable bank
    VerifyMemoryValue(0x3FFF, 0x00, "End of bank 0");
    VerifyMemoryValue(0x4000, 0x01, "Start of switchable bank");
    
    // Test boundary within switchable bank area
    VerifyMemoryValue(0x7FFF, 0x01, "End of switchable bank");
    
    // Test ROM/RAM boundary
    _bus->Write(0x0000, 0x0A);
    _bus->Write(0xA000, 0x55);
    VerifyMemoryValue(0xA000, 0x55, "Read from start of RAM");
    VerifyMemoryValue(0x9FFF, 0x00, "VRAM area should not be affected");
    
    LOG("    Bank boundaries test passed!");
}

void Mbc5Test::TestInvalidBankAccess()
{
    LOG("    Testing invalid bank access...");
    
    // Test accessing beyond available ROM banks
    // Our test ROM has 32 banks, so bank 32+ should wrap
    _bus->Write(0x2000, 0x20);
    VerifyMemoryValue(0x4000, 0x00, "Should wrap to bank 0");
    
    // Test invalid RAM bank (our test has 16 banks, so 16+ should wrap)
    _bus->Write(0x0000, 0x0A);
    _bus->Write(0x4000, 0x10);
    _bus->Write(0xA000, 0x77);
    _bus->Write(0x4000, 0x00);
    VerifyMemoryValue(0xA000, 0x77, "Should read from wrapped bank (bank 0)");
    
    LOG("    Invalid bank access test passed!");
}

std::vector<byte> Mbc5Test::CreateMbc5Rom(int numBanks, int ramSize)
{
    // Calculate ROM size (each bank is 16KB)
    int romSizeBytes = numBanks * GbConstants::RomBankSize;
    std::vector<byte> rom(romSizeBytes, 0x00);
    
    // Fill each bank with identifiable data (bank number)
    for (int bank = 0; bank < numBanks; bank++)
    {
        int bankOffset = bank * GbConstants::RomBankSize;
        for (int i = 0; i < GbConstants::RomBankSize; i++)
        {
            rom[bankOffset + i] = static_cast<byte>(bank);
        }
    }
    
    // Set up MBC5 cartridge header
    rom[AddressConstants::CartridgeTypeAddress] = 0x19; // MBC5
    
    // Set ROM size (log2 calculation for standard ROM sizes)
    byte romSizeFlag = 0;
    int tempSize = numBanks;
    while (tempSize > 2) {
        tempSize >>= 1;
        romSizeFlag++;
    }
    rom[AddressConstants::CartridgeRomSizeAddress] = romSizeFlag;
    
    // Set RAM size
    rom[AddressConstants::CartridgeRamSizeAddress] = ramSize;
    
    return rom;
}

void Mbc5Test::VerifyMemoryValue(word address, byte expected, const std::string& message)
{
    byte actual = _bus->Read(address);
    if (actual != expected)
    {
        std::ostringstream oss;
        oss << message << " - Expected: 0x" << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(expected) << ", Got: 0x" << static_cast<int>(actual);
        throw std::runtime_error(oss.str());
    }
}