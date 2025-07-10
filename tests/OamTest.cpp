#include "OamTest.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/MBC/NoMbc.h"
#include "Emulator/GbConstants.h"
#include <stdexcept>

OamTest::OamTest() : BaseTest("OAM Test")
{
}

void OamTest::Setup()
{
    LOG("  Setting up OAM test...");
    
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
    
    LOG("  OAM test setup complete!");
}

void OamTest::Run()
{
    LOG("  Testing OAM functionality...");
    
    TestOamBasicReadWrite();
    TestOamAddressTranslation();
    TestOamBoundaryConditions();
    TestOamSpriteAttributes();
    TestOamSpritePositions();
    TestOamTileIndices();
    TestOamAttributeFlags();
    TestOamConstants();
    
    LOG("  OAM test completed successfully!");
}

void OamTest::TestOamBasicReadWrite()
{
    LOG("    Testing basic OAM read/write...");
    
    // Test basic read/write operations
    _bus->Write(0xFE00, 0x42);
    VerifyOamValue(0xFE00, 0x42, "Basic OAM write/read at 0xFE00");
    
    _bus->Write(0xFE01, 0x55);
    VerifyOamValue(0xFE01, 0x55, "Basic OAM write/read at 0xFE01");
    
    // Test that previous value is preserved
    VerifyOamValue(0xFE00, 0x42, "Previous OAM value preservation");
    
    // Test different values
    _bus->Write(0xFE50, 0xAA);
    VerifyOamValue(0xFE50, 0xAA, "Basic OAM write/read at 0xFE50");
    
    _bus->Write(0xFE9F, 0x33);
    VerifyOamValue(0xFE9F, 0x33, "Basic OAM write/read at 0xFE9F");
    
    LOG("    ✓ Basic OAM read/write test passed");
}

void OamTest::TestOamAddressTranslation()
{
    LOG("    Testing OAM address translation...");
    
    // Test that OAM addresses are properly translated
    // OAM starts at 0xFE00, so 0xFE00 -> internal address 0x0000
    _bus->Write(0xFE00, 0x11);
    VerifyOamValue(0xFE00, 0x11, "OAM address translation 0xFE00");
    
    // Test mid-range address
    _bus->Write(0xFE50, 0x22);
    VerifyOamValue(0xFE50, 0x22, "OAM address translation 0xFE50");
    
    // Test end address
    _bus->Write(0xFE9F, 0x33);
    VerifyOamValue(0xFE9F, 0x33, "OAM address translation 0xFE9F");
    
    // Verify they don't interfere with each other
    VerifyOamValue(0xFE00, 0x11, "OAM address isolation 0xFE00");
    VerifyOamValue(0xFE50, 0x22, "OAM address isolation 0xFE50");
    
    LOG("    ✓ OAM address translation test passed");
}

void OamTest::TestOamBoundaryConditions()
{
    LOG("    Testing OAM boundary conditions...");
    
    // Test start boundary
    _bus->Write(0xFE00, 0x44);
    VerifyOamValue(0xFE00, 0x44, "OAM start boundary 0xFE00");
    
    // Test end boundary
    _bus->Write(0xFE9F, 0x55);
    VerifyOamValue(0xFE9F, 0x55, "OAM end boundary 0xFE9F");
    
    // Test just before start (should not affect OAM)
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing OAM boundary at 0xFDFF (intentional error expected)...");
    _bus->Write(0xFDFF, 0x66);
    LOG("    OAM boundary test at 0xFDFF completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    VerifyOamValue(0xFE00, 0x44, "OAM isolation from 0xFDFF");
    
    // Test just after end (0xFEA0 is not used area)
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing OAM boundary at 0xFEA0 (intentional error expected)...");
    _bus->Write(0xFEA0, 0x77);
    LOG("    OAM boundary test at 0xFEA0 completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    VerifyOamValue(0xFE9F, 0x55, "OAM isolation from 0xFEA0");
    
    LOG("    ✓ OAM boundary conditions test passed");
}

void OamTest::TestOamSpriteAttributes()
{
    LOG("    Testing OAM sprite attributes...");
    
    // Test sprite data structure (4 bytes per sprite)
    // Sprite 0 at 0xFE00-0xFE03
    _bus->Write(0xFE00, 0x10); // Y position
    _bus->Write(0xFE01, 0x08); // X position
    _bus->Write(0xFE02, 0x42); // Tile index
    _bus->Write(0xFE03, 0x00); // Attributes
    
    VerifyOamValue(0xFE00, 0x10, "Sprite 0 Y position");
    VerifyOamValue(0xFE01, 0x08, "Sprite 0 X position");
    VerifyOamValue(0xFE02, 0x42, "Sprite 0 tile index");
    VerifyOamValue(0xFE03, 0x00, "Sprite 0 attributes");
    
    // Test sprite 1 at 0xFE04-0xFE07
    _bus->Write(0xFE04, 0x20); // Y position
    _bus->Write(0xFE05, 0x10); // X position
    _bus->Write(0xFE06, 0x43); // Tile index
    _bus->Write(0xFE07, 0x20); // Attributes (X flip)
    
    VerifyOamValue(0xFE04, 0x20, "Sprite 1 Y position");
    VerifyOamValue(0xFE05, 0x10, "Sprite 1 X position");
    VerifyOamValue(0xFE06, 0x43, "Sprite 1 tile index");
    VerifyOamValue(0xFE07, 0x20, "Sprite 1 attributes");
    
    // Verify sprite 0 is still intact
    VerifyOamValue(0xFE00, 0x10, "Sprite 0 Y position preserved");
    VerifyOamValue(0xFE01, 0x08, "Sprite 0 X position preserved");
    
    LOG("    ✓ OAM sprite attributes test passed");
}

void OamTest::TestOamSpritePositions()
{
    LOG("    Testing OAM sprite positions...");
    
    // Test various sprite positions
    // Game Boy sprites have position offsets (Y+16, X+8)
    
    // Sprite at screen position (0, 0) would be stored as (16, 8)
    _bus->Write(0xFE08, 16); // Y position for screen Y=0
    _bus->Write(0xFE09, 8);  // X position for screen X=0
    
    VerifyOamValue(0xFE08, 16, "Sprite screen position Y=0");
    VerifyOamValue(0xFE09, 8, "Sprite screen position X=0");
    
    // Sprite at screen position (100, 80) would be stored as (116, 88)
    _bus->Write(0xFE0C, 116); // Y position for screen Y=100
    _bus->Write(0xFE0D, 88);  // X position for screen X=80
    
    VerifyOamValue(0xFE0C, 116, "Sprite screen position Y=100");
    VerifyOamValue(0xFE0D, 88, "Sprite screen position X=80");
    
    // Test edge positions
    _bus->Write(0xFE10, 0);   // Y position 0 (off-screen top)
    _bus->Write(0xFE11, 0);   // X position 0 (off-screen left)
    _bus->Write(0xFE14, 255); // Y position 255 (off-screen bottom)
    _bus->Write(0xFE15, 255); // X position 255 (off-screen right)
    
    VerifyOamValue(0xFE10, 0, "Sprite edge position Y=0");
    VerifyOamValue(0xFE11, 0, "Sprite edge position X=0");
    VerifyOamValue(0xFE14, 255, "Sprite edge position Y=255");
    VerifyOamValue(0xFE15, 255, "Sprite edge position X=255");
    
    LOG("    ✓ OAM sprite positions test passed");
}

void OamTest::TestOamTileIndices()
{
    LOG("    Testing OAM tile indices...");
    
    // Test various tile indices
    // Game Boy has 256 tiles (0x00-0xFF)
    
    // Test tile index 0 (first tile)
    _bus->Write(0xFE18, 0x00); // Sprite Y
    _bus->Write(0xFE19, 0x00); // Sprite X
    _bus->Write(0xFE1A, 0x00); // Tile index 0
    
    VerifyOamValue(0xFE1A, 0x00, "Tile index 0");
    
    // Test tile index 255 (last tile)
    _bus->Write(0xFE1C, 0x00); // Sprite Y
    _bus->Write(0xFE1D, 0x00); // Sprite X
    _bus->Write(0xFE1E, 0xFF); // Tile index 255
    
    VerifyOamValue(0xFE1E, 0xFF, "Tile index 255");
    
    // Test common tile indices
    _bus->Write(0xFE22, 0x01); // Tile index 1
    _bus->Write(0xFE26, 0x10); // Tile index 16
    _bus->Write(0xFE2A, 0x80); // Tile index 128
    
    VerifyOamValue(0xFE22, 0x01, "Tile index 1");
    VerifyOamValue(0xFE26, 0x10, "Tile index 16");
    VerifyOamValue(0xFE2A, 0x80, "Tile index 128");
    
    LOG("    ✓ OAM tile indices test passed");
}

void OamTest::TestOamAttributeFlags()
{
    LOG("    Testing OAM attribute flags...");
    
    // Test attribute flags (bit 7-0):
    // Bit 7: BG and Window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
    // Bit 6: Y flip (0=Normal, 1=Vertically mirrored)
    // Bit 5: X flip (0=Normal, 1=Horizontally mirrored)
    // Bit 4: Palette number (0=OBP0, 1=OBP1)
    // Bit 3: Not used
    // Bit 2-0: Not used
    
    // Test no flags (normal sprite)
    _bus->Write(0xFE2B, 0x00); // No flags
    VerifyOamValue(0xFE2B, 0x00, "Attribute flags: none");
    
    // Test palette selection
    _bus->Write(0xFE2F, 0x10); // Palette 1
    VerifyOamValue(0xFE2F, 0x10, "Attribute flags: palette 1");
    
    // Test X flip
    _bus->Write(0xFE33, 0x20); // X flip
    VerifyOamValue(0xFE33, 0x20, "Attribute flags: X flip");
    
    // Test Y flip
    _bus->Write(0xFE37, 0x40); // Y flip
    VerifyOamValue(0xFE37, 0x40, "Attribute flags: Y flip");
    
    // Test BG priority
    _bus->Write(0xFE3B, 0x80); // BG priority
    VerifyOamValue(0xFE3B, 0x80, "Attribute flags: BG priority");
    
    // Test combined flags
    _bus->Write(0xFE3F, 0xF0); // All flags
    VerifyOamValue(0xFE3F, 0xF0, "Attribute flags: all combined");
    
    LOG("    ✓ OAM attribute flags test passed");
}

void OamTest::TestOamConstants()
{
    LOG("    Testing OAM constants...");
    
    // Test that OAM constants are correct
    if (AddressConstants::StartOamAddress != 0xFE00)
    {
        throw std::runtime_error("OAM StartOamAddress constant incorrect");
    }
    if (AddressConstants::EndOamAddress != 0xFE9F)
    {
        throw std::runtime_error("OAM EndOamAddress constant incorrect");
    }
    
    // Test OAM size calculation
    word expectedSize = AddressConstants::EndOamAddress - AddressConstants::StartOamAddress + 1;
    if (expectedSize != 0xA0) // 160 bytes
    {
        throw std::runtime_error("OAM size calculation incorrect");
    }
    
    // Test that OAM can hold exactly 40 sprites (160 bytes / 4 bytes per sprite)
    if (expectedSize / 4 != 40)
    {
        throw std::runtime_error("OAM should hold exactly 40 sprites");
    }
    
    // Test that 0xFEA0 is not part of OAM (it's not used area)
    if (AddressConstants::EndOamAddress >= 0xFEA0)
    {
        throw std::runtime_error("OAM should not include 0xFEA0 (not used area)");
    }
    
    LOG("    ✓ OAM constants test passed");
}

void OamTest::VerifyOamValue(word address, byte expected, const std::string& testName)
{
    byte actual = _bus->Read(address);
    if (actual == expected) {
        LOG("    ✓ " + testName + " - expected 0x" + std::to_string(static_cast<int>(expected)) + ", got 0x" + std::to_string(static_cast<int>(actual)));
    } else {
        LOG("    ✗ " + testName + " - expected 0x" + std::to_string(static_cast<int>(expected)) + ", got 0x" + std::to_string(static_cast<int>(actual)));
        throw std::runtime_error(testName + " test failed");
    }
}

void OamTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size() && i < _cartridgeData.size(); i++)
    {
        _cartridgeData[i] = program[i];
    }
}

void OamTest::ExecuteInstructions(int count)
{
    for (int i = 0; i < count; i++)
    {
        _cpu->Update();
    }
}