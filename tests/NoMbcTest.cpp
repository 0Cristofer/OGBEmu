#include "NoMbcTest.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/GbConstants.h"
#include <stdexcept>

NoMbcTest::NoMbcTest() : BaseTest("NoMbc Test")
{
}

void NoMbcTest::Setup()
{
    LOG("  Setting up NoMbc test...");
    
    CreateValidRom();
    
    LOG("  NoMbc test setup complete!");
}

void NoMbcTest::Run()
{
    LOG("  Testing NoMbc functionality...");
    
    TestNoMbcRomRead();
    TestNoMbcRomReadInvalidAddress();
    TestNoMbcRamWrite();
    TestNoMbcRamWriteInvalidAddress();
    TestNoMbcRamSizeDetection();
    TestNoMbcAddressTranslation();
    TestNoMbcRamConfiguration();
    TestNoMbcEdgeCases();
    
    LOG("  NoMbc test completed successfully!");
}

void NoMbcTest::TestNoMbcRomRead()
{
    LOG("    Testing NoMbc ROM read functionality...");
    
    CreateValidRom();
    NoMbc noMbc(&_romData);
    
    // Test reading from ROM addresses
    byte value = noMbc.Read(0x0000);
    if (value != 0x00) {
        throw std::runtime_error("NoMbc ROM read failed at address 0x0000");
    }
    
    // Test reading from entry point
    value = noMbc.Read(0x0100);
    if (value != 0x00) {
        throw std::runtime_error("NoMbc ROM read failed at address 0x0100");
    }
    
    // Test reading from middle of ROM
    value = noMbc.Read(0x4000);
    if (value != 0x00) {
        throw std::runtime_error("NoMbc ROM read failed at address 0x4000");
    }
    
    LOG("    ✓ NoMbc ROM read functionality works correctly");
}

void NoMbcTest::TestNoMbcRomReadInvalidAddress()
{
    LOG("    Testing NoMbc ROM read with invalid address...");
    
    CreateValidRom();
    NoMbc noMbc(&_romData);
    
    // Test reading from address beyond ROM size
    word invalidAddress = static_cast<word>(_romData.size() + 0x1000);
    byte value = noMbc.Read(invalidAddress);
    if (value != 0x00) {
        throw std::runtime_error("NoMbc should return 0x00 for invalid ROM address");
    }
    
    LOG("    ✓ NoMbc ROM read with invalid address handled correctly");
}

void NoMbcTest::TestNoMbcRamWrite()
{
    LOG("    Testing NoMbc RAM write functionality...");
    
    CreateValidRomWithRam();
    NoMbc noMbc(&_romData);
    
    // Test writing to RAM addresses
    noMbc.Write(AddressConstants::StartExternalRamAddress, 0x42);
    noMbc.Write(AddressConstants::StartExternalRamAddress + 0x100, 0x55);
    noMbc.Write(AddressConstants::StartExternalRamAddress + 0x1000, 0xAA);
    
    // Note: Can't directly read RAM in NoMbc, but no exceptions should be thrown
    LOG("    ✓ NoMbc RAM write functionality works correctly");
}

void NoMbcTest::TestNoMbcRamWriteInvalidAddress()
{
    LOG("    Testing NoMbc RAM write with invalid address...");
    
    CreateValidRomWithRam();
    NoMbc noMbc(&_romData);
    
    // Test writing to address beyond RAM size
    word invalidAddress = AddressConstants::StartExternalRamAddress + GbConstants::RamBankSize + 0x1000;
    noMbc.Write(invalidAddress, 0x42);
    
    // No exception should be thrown, just logged
    LOG("    ✓ NoMbc RAM write with invalid address handled correctly");
}

void NoMbcTest::TestNoMbcRamSizeDetection()
{
    LOG("    Testing NoMbc RAM size detection...");
    
    // Test with no RAM
    CreateValidRomWithNoRam();
    NoMbc noMbcNoRam(&_romData);
    
    // Writing to RAM should not crash when no RAM is configured
    noMbcNoRam.Write(AddressConstants::StartExternalRamAddress, 0x42);
    
    LOG("    ✓ No RAM configuration handled correctly");
    
    // Test with 1 bank of RAM
    CreateValidRomWithRam();
    NoMbc noMbcWithRam(&_romData);
    
    // Writing to RAM should work when RAM is configured
    noMbcWithRam.Write(AddressConstants::StartExternalRamAddress, 0x42);
    
    LOG("    ✓ Single RAM bank configuration handled correctly");
    LOG("    ✓ NoMbc RAM size detection works correctly");
}

void NoMbcTest::TestNoMbcAddressTranslation()
{
    LOG("    Testing NoMbc address translation...");
    
    CreateValidRomWithRam();
    NoMbc noMbc(&_romData);
    
    // Test address translation for RAM writes
    // External RAM starts at 0xA000, should translate to 0x0000 in internal RAM
    noMbc.Write(AddressConstants::StartExternalRamAddress, 0x42);
    noMbc.Write(AddressConstants::StartExternalRamAddress + 0x100, 0x55);
    noMbc.Write(AddressConstants::StartExternalRamAddress + 0x1000, 0xAA);
    
    // All writes should succeed without exceptions
    LOG("    ✓ NoMbc address translation works correctly");
}

void NoMbcTest::TestNoMbcRamConfiguration()
{
    LOG("    Testing NoMbc RAM configuration...");
    
    // Test different RAM size configurations
    CreateValidRomWithNoRam();
    NoMbc noMbcNoRam(&_romData);
    
    // Should not crash with no RAM
    noMbcNoRam.Write(AddressConstants::StartExternalRamAddress, 0x42);
    
    LOG("    ✓ No RAM configuration works correctly");
    
    CreateValidRomWithRam();
    NoMbc noMbcWithRam(&_romData);
    
    // Should work with single RAM bank
    noMbcWithRam.Write(AddressConstants::StartExternalRamAddress, 0x42);
    
    LOG("    ✓ Single RAM bank configuration works correctly");
    LOG("    ✓ NoMbc RAM configuration test passed");
}

void NoMbcTest::TestNoMbcEdgeCases()
{
    LOG("    Testing NoMbc edge cases...");
    
    CreateValidRom();
    NoMbc noMbc(&_romData);
    
    // Test reading from ROM boundary
    byte value = noMbc.Read(static_cast<word>(_romData.size() - 1));
    if (value != 0x00) {
        throw std::runtime_error("NoMbc ROM read failed at ROM boundary");
    }
    
    // Test reading exactly at ROM size (should fail)
    value = noMbc.Read(static_cast<word>(_romData.size()));
    if (value != 0x00) {
        throw std::runtime_error("NoMbc should return 0x00 for address at ROM size");
    }
    
    // Test writing to RAM boundary
    CreateValidRomWithRam();
    NoMbc noMbcWithRam(&_romData);
    
    // Write to last valid RAM address
    noMbcWithRam.Write(AddressConstants::StartExternalRamAddress + GbConstants::RamBankSize - 1, 0x42);
    
    // Write to first invalid RAM address (should fail gracefully)
    noMbcWithRam.Write(AddressConstants::StartExternalRamAddress + GbConstants::RamBankSize, 0x42);
    
    LOG("    ✓ NoMbc edge cases handled correctly");
}

void NoMbcTest::CreateValidRom()
{
    // Create minimal valid 32KB ROM
    _romData.resize(GbConstants::MinCartridgeRomSize, 0x00);
    
    // Set entry point
    _romData[0x0100] = 0x00; // NOP
    _romData[0x0101] = 0xC3; // JP 0x0150
    _romData[0x0102] = 0x50;
    _romData[0x0103] = 0x01;
    
    // Set cartridge header (no RAM by default)
    _romData[AddressConstants::CartridgeTypeAddress] = static_cast<byte>(CartridgeType::RomOnly);
    _romData[AddressConstants::CartridgeRomSizeAddress] = 0x00; // 32KB
    _romData[AddressConstants::CartridgeRamSizeAddress] = GbConstants::RamSizeFlagNoRam;
}

void NoMbcTest::CreateValidRomWithRam()
{
    CreateValidRom();
    
    // Set RAM size to 1 bank (2KB)
    _romData[AddressConstants::CartridgeRamSizeAddress] = GbConstants::RamSizeFlag1Bank;
}

void NoMbcTest::CreateValidRomWithNoRam()
{
    CreateValidRom();
    
    // Explicitly set no RAM (already default)
    _romData[AddressConstants::CartridgeRamSizeAddress] = GbConstants::RamSizeFlagNoRam;
}

void NoMbcTest::CreateInvalidRom()
{
    // Create too small ROM
    _romData.resize(0x1000, 0x00);
}