#include "VRamTest.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/MBC/NoMbc.h"
#include "Emulator/GbConstants.h"
#include <stdexcept>

VRamTest::VRamTest() : BaseTest("VRam Test")
{
}

void VRamTest::Setup()
{
    LOG("  Setting up VRam test...");
    
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
    
    LOG("  VRam test setup complete!");
}

void VRamTest::Run()
{
    LOG("  Testing VRam functionality...");
    
    TestVRamBasicReadWrite();
    TestVRamAddressTranslation();
    TestVRamBoundaryConditions();
    TestVRamPatternStorage();
    TestVRamTileDataAccess();
    TestVRamBankingSwitching();
    TestVRamConstants();
    
    LOG("  VRam test completed successfully!");
}

void VRamTest::TestVRamBasicReadWrite()
{
    LOG("    Testing basic VRam read/write...");
    
    // Test basic read/write operations
    _bus->Write(0x8000, 0x42);
    VerifyVRamValue(0x8000, 0x42, "Basic VRam write/read at 0x8000");
    
    _bus->Write(0x8001, 0x55);
    VerifyVRamValue(0x8001, 0x55, "Basic VRam write/read at 0x8001");
    
    // Test that previous value is preserved
    VerifyVRamValue(0x8000, 0x42, "Previous VRam value preservation");
    
    // Test different values
    _bus->Write(0x9000, 0xAA);
    VerifyVRamValue(0x9000, 0xAA, "Basic VRam write/read at 0x9000");
    
    _bus->Write(0x9FFF, 0x33);
    VerifyVRamValue(0x9FFF, 0x33, "Basic VRam write/read at 0x9FFF");
    
    LOG("    ✓ Basic VRam read/write test passed");
}

void VRamTest::TestVRamAddressTranslation()
{
    LOG("    Testing VRam address translation...");
    
    // Test that VRam addresses are properly translated
    // VRam starts at 0x8000, so 0x8000 -> internal address 0x0000
    _bus->Write(0x8000, 0x11);
    VerifyVRamValue(0x8000, 0x11, "VRam address translation 0x8000");
    
    // Test mid-range address
    _bus->Write(0x8800, 0x22);
    VerifyVRamValue(0x8800, 0x22, "VRam address translation 0x8800");
    
    // Test end address
    _bus->Write(0x9FFF, 0x33);
    VerifyVRamValue(0x9FFF, 0x33, "VRam address translation 0x9FFF");
    
    // Verify they don't interfere with each other
    VerifyVRamValue(0x8000, 0x11, "VRam address isolation 0x8000");
    VerifyVRamValue(0x8800, 0x22, "VRam address isolation 0x8800");
    
    LOG("    ✓ VRam address translation test passed");
}

void VRamTest::TestVRamBoundaryConditions()
{
    LOG("    Testing VRam boundary conditions...");
    
    // Test start boundary
    _bus->Write(0x8000, 0x44);
    VerifyVRamValue(0x8000, 0x44, "VRam start boundary 0x8000");
    
    // Test end boundary
    _bus->Write(0x9FFF, 0x55);
    VerifyVRamValue(0x9FFF, 0x55, "VRam end boundary 0x9FFF");
    
    // Test just before start (should not affect VRam)
    _bus->Write(0x7FFF, 0x66);
    VerifyVRamValue(0x8000, 0x44, "VRam isolation from 0x7FFF");
    
    // Test just after end (should not affect VRam)
    _bus->Write(0xA000, 0x77);
    VerifyVRamValue(0x9FFF, 0x55, "VRam isolation from 0xA000");
    
    LOG("    ✓ VRam boundary conditions test passed");
}

void VRamTest::TestVRamPatternStorage()
{
    LOG("    Testing VRam pattern storage...");
    
    // Test storing various patterns
    const std::vector<byte> pattern = {0x00, 0x18, 0x24, 0x42, 0x42, 0x24, 0x18, 0x00};
    
    // Write pattern to VRam
    word baseAddr = 0x8000;
    for (size_t i = 0; i < pattern.size(); i++)
    {
        _bus->Write(baseAddr + i, pattern[i]);
    }
    
    // Verify pattern was stored correctly
    for (size_t i = 0; i < pattern.size(); i++)
    {
        VerifyVRamValue(baseAddr + i, pattern[i], "VRam pattern storage at offset " + std::to_string(i));
    }
    
    // Test another pattern at different location
    const std::vector<byte> pattern2 = {0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF};
    baseAddr = 0x8010;
    for (size_t i = 0; i < pattern2.size(); i++)
    {
        _bus->Write(baseAddr + i, pattern2[i]);
    }
    
    // Verify second pattern
    for (size_t i = 0; i < pattern2.size(); i++)
    {
        VerifyVRamValue(baseAddr + i, pattern2[i], "VRam pattern2 storage at offset " + std::to_string(i));
    }
    
    // Verify first pattern is still intact
    baseAddr = 0x8000;
    for (size_t i = 0; i < pattern.size(); i++)
    {
        VerifyVRamValue(baseAddr + i, pattern[i], "VRam pattern1 preservation at offset " + std::to_string(i));
    }
    
    LOG("    ✓ VRam pattern storage test passed");
}

void VRamTest::TestVRamTileDataAccess()
{
    LOG("    Testing VRam tile data access...");
    
    // Test tile data area (0x8000-0x8FFF)
    _bus->Write(0x8000, 0x3C); // Tile 0, row 0, low byte
    _bus->Write(0x8001, 0x42); // Tile 0, row 0, high byte
    
    VerifyVRamValue(0x8000, 0x3C, "VRam tile data low byte");
    VerifyVRamValue(0x8001, 0x42, "VRam tile data high byte");
    
    // Test tile map area (0x9800-0x9BFF)
    _bus->Write(0x9800, 0x01); // First tile map entry
    _bus->Write(0x9801, 0x02); // Second tile map entry
    
    VerifyVRamValue(0x9800, 0x01, "VRam tile map entry 1");
    VerifyVRamValue(0x9801, 0x02, "VRam tile map entry 2");
    
    // Test alternate tile map area (0x9C00-0x9FFF)
    _bus->Write(0x9C00, 0x03); // First tile map entry (alternate)
    _bus->Write(0x9C01, 0x04); // Second tile map entry (alternate)
    
    VerifyVRamValue(0x9C00, 0x03, "VRam alternate tile map entry 1");
    VerifyVRamValue(0x9C01, 0x04, "VRam alternate tile map entry 2");
    
    // Verify areas don't interfere
    VerifyVRamValue(0x8000, 0x3C, "VRam tile data isolation");
    VerifyVRamValue(0x9800, 0x01, "VRam tile map isolation");
    
    LOG("    ✓ VRam tile data access test passed");
}

void VRamTest::TestVRamBankingSwitching()
{
    LOG("    Testing VRam banking switching...");
    
    // For DMG (original Game Boy), there's only one VRam bank
    // This test verifies that banking is handled correctly
    
    // Write to VRam
    _bus->Write(0x8000, 0x88);
    VerifyVRamValue(0x8000, 0x88, "VRam banking single bank access");
    
    // Write to different address
    _bus->Write(0x9000, 0x99);
    VerifyVRamValue(0x9000, 0x99, "VRam banking different address");
    
    // Verify isolation
    VerifyVRamValue(0x8000, 0x88, "VRam banking isolation");
    
    LOG("    ✓ VRam banking switching test passed");
}

void VRamTest::TestVRamConstants()
{
    LOG("    Testing VRam constants...");
    
    // Test that VRam constants are correct
    if (AddressConstants::StartVRamAddress != 0x8000)
    {
        throw std::runtime_error("VRam StartVRamAddress constant incorrect");
    }
    if (AddressConstants::EndVRamAddress != 0x9FFF)
    {
        throw std::runtime_error("VRam EndVRamAddress constant incorrect");
    }
    
    // Test VRam size calculation
    word expectedSize = AddressConstants::EndVRamAddress - AddressConstants::StartVRamAddress + 1;
    if (expectedSize != 0x2000) // 8KB
    {
        throw std::runtime_error("VRam size calculation incorrect");
    }
    
    LOG("    ✓ VRam constants test passed");
}

void VRamTest::VerifyVRamValue(word address, byte expected, const std::string& testName)
{
    byte actual = _bus->Read(address);
    if (actual == expected) {
        LOG("    ✓ " + testName + " - expected 0x" + std::to_string(expected) + ", got 0x" + std::to_string(actual));
    } else {
        LOG("    ✗ " + testName + " - expected 0x" + std::to_string(expected) + ", got 0x" + std::to_string(actual));
        throw std::runtime_error(testName + " test failed");
    }
}

void VRamTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size() && i < _cartridgeData.size(); i++)
    {
        _cartridgeData[i] = program[i];
    }
}

void VRamTest::ExecuteInstructions(int count)
{
    for (int i = 0; i < count; i++)
    {
        _cpu->Update();
    }
}