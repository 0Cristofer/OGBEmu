#include "WRamTest.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Emulator/Memory/MBC/NoMbc.h"
#include "Emulator/GbConstants.h"
#include <stdexcept>

WRamTest::WRamTest() : BaseTest("WRam Test")
{
}

void WRamTest::Setup()
{
    LOG("  Setting up WRam test...");
    
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
    
    LOG("  WRam test setup complete!");
}

void WRamTest::Run()
{
    LOG("  Testing WRam functionality...");
    
    TestWRamBasicReadWrite();
    TestWRamAddressTranslation();
    TestWRamBoundaryConditions();
    TestWRamProgramStorage();
    TestWRamStackOperations();
    TestWRamVariableStorage();
    TestWRamConstants();
    
    LOG("  WRam test completed successfully!");
}

void WRamTest::TestWRamBasicReadWrite()
{
    LOG("    Testing basic WRam read/write...");
    
    // Test basic read/write operations
    _bus->Write(0xC000, 0x42);
    VerifyWRamValue(0xC000, 0x42, "Basic WRam write/read at 0xC000");
    
    _bus->Write(0xC001, 0x55);
    VerifyWRamValue(0xC001, 0x55, "Basic WRam write/read at 0xC001");
    
    // Test that previous value is preserved
    VerifyWRamValue(0xC000, 0x42, "Previous WRam value preservation");
    
    // Test different values
    _bus->Write(0xC800, 0xAA);
    VerifyWRamValue(0xC800, 0xAA, "Basic WRam write/read at 0xC800");
    
    _bus->Write(0xCFFF, 0x33);
    VerifyWRamValue(0xCFFF, 0x33, "Basic WRam write/read at 0xCFFF");
    
    LOG("    ✓ Basic WRam read/write test passed");
}

void WRamTest::TestWRamAddressTranslation()
{
    LOG("    Testing WRam address translation...");
    
    // Test that WRam addresses are properly translated
    // WRam starts at 0xC000, so 0xC000 -> internal address 0x0000
    _bus->Write(0xC000, 0x11);
    VerifyWRamValue(0xC000, 0x11, "WRam address translation 0xC000");
    
    // Test mid-range address
    _bus->Write(0xC800, 0x22);
    VerifyWRamValue(0xC800, 0x22, "WRam address translation 0xC800");
    
    // Test end address
    _bus->Write(0xCFFF, 0x33);
    VerifyWRamValue(0xCFFF, 0x33, "WRam address translation 0xCFFF");
    
    // Verify they don't interfere with each other
    VerifyWRamValue(0xC000, 0x11, "WRam address isolation 0xC000");
    VerifyWRamValue(0xC800, 0x22, "WRam address isolation 0xC800");
    
    LOG("    ✓ WRam address translation test passed");
}

void WRamTest::TestWRamBoundaryConditions()
{
    LOG("    Testing WRam boundary conditions...");
    
    // Test start boundary
    _bus->Write(0xC000, 0x44);
    VerifyWRamValue(0xC000, 0x44, "WRam start boundary 0xC000");
    
    // Test end boundary
    _bus->Write(0xCFFF, 0x55);
    VerifyWRamValue(0xCFFF, 0x55, "WRam end boundary 0xCFFF");
    
    // Test just before start (should not affect WRam)
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing WRam boundary at 0xBFFF (intentional error expected)...");
    _bus->Write(0xBFFF, 0x66);
    LOG("    WRam boundary test at 0xBFFF completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    VerifyWRamValue(0xC000, 0x44, "WRam isolation from 0xBFFF");
    
    // Test just after end (should not affect WRam)
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing WRam boundary at 0xD000 (intentional error expected)...");
    _bus->Write(0xD000, 0x77);
    LOG("    WRam boundary test at 0xD000 completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    VerifyWRamValue(0xCFFF, 0x55, "WRam isolation from 0xD000");
    
    LOG("    ✓ WRam boundary conditions test passed");
}

void WRamTest::TestWRamProgramStorage()
{
    LOG("    Testing WRam program storage...");
    
    // Test storing program data in WRam
    const std::vector<byte> program = {
        0x3E, 0x42,       // LD A, 0x42
        0x06, 0x10,       // LD B, 0x10
        0x0E, 0x20,       // LD C, 0x20
        0x16, 0x30,       // LD D, 0x30
        0x1E, 0x40        // LD E, 0x40
    };
    
    // Write program to WRam
    word baseAddr = 0xC000;
    for (size_t i = 0; i < program.size(); i++)
    {
        _bus->Write(baseAddr + static_cast<word>(i), program[i]);
    }
    
    // Verify program was stored correctly
    for (size_t i = 0; i < program.size(); i++)
    {
        VerifyWRamValue(baseAddr + static_cast<word>(i), program[i], "WRam program storage at offset " + std::to_string(i));
    }
    
    // Test another program at different location
    const std::vector<byte> program2 = {0x76, 0x00, 0x18, 0xFE}; // HALT, NOP, JR -2
    baseAddr = 0xC100;
    for (size_t i = 0; i < program2.size(); i++)
    {
        _bus->Write(baseAddr + static_cast<word>(i), program2[i]);
    }
    
    // Verify second program
    for (size_t i = 0; i < program2.size(); i++)
    {
        VerifyWRamValue(baseAddr + static_cast<word>(i), program2[i], "WRam program2 storage at offset " + std::to_string(i));
    }
    
    // Verify first program is still intact
    baseAddr = 0xC000;
    for (size_t i = 0; i < program.size(); i++)
    {
        VerifyWRamValue(baseAddr + static_cast<word>(i), program[i], "WRam program1 preservation at offset " + std::to_string(i));
    }
    
    LOG("    ✓ WRam program storage test passed");
}

void WRamTest::TestWRamStackOperations()
{
    LOG("    Testing WRam stack operations...");
    
    // Test stack data in WRam (stack typically grows downward from 0xFFFE)
    // But we'll test WRam portion of stack space
    
    // Test storing stack-like data
    word stackPtr = 0xCFFE; // High end of WRam
    
    // Simulate PUSH operations (decrement then store)
    _bus->Write(stackPtr, 0x12);
    _bus->Write(stackPtr - 1, 0x34);
    stackPtr -= 2;
    
    // Verify stack data
    VerifyWRamValue(0xCFFE, 0x12, "WRam stack high byte");
    VerifyWRamValue(0xCFFD, 0x34, "WRam stack low byte");
    
    // Test more stack operations
    _bus->Write(stackPtr, 0x56);
    _bus->Write(stackPtr - 1, 0x78);
    stackPtr -= 2;
    
    VerifyWRamValue(0xCFFC, 0x56, "WRam stack second high byte");
    VerifyWRamValue(0xCFFB, 0x78, "WRam stack second low byte");
    
    // Verify previous data is preserved
    VerifyWRamValue(0xCFFE, 0x12, "WRam stack preservation 1");
    VerifyWRamValue(0xCFFD, 0x34, "WRam stack preservation 2");
    
    LOG("    ✓ WRam stack operations test passed");
}

void WRamTest::TestWRamVariableStorage()
{
    LOG("    Testing WRam variable storage...");
    
    // Test storing various data types in WRam
    word varBase = 0xC200;
    
    // Store different variable types
    _bus->Write(varBase, 0x01);         // Boolean true
    _bus->Write(varBase + 1, 0x00);     // Boolean false
    _bus->Write(varBase + 2, 0xFF);     // Byte max value
    _bus->Write(varBase + 3, 0x80);     // Signed byte -128
    _bus->Write(varBase + 4, 0x7F);     // Signed byte 127
    
    // Store 16-bit values (little-endian)
    _bus->Write(varBase + 5, 0x34);     // Low byte of 0x1234
    _bus->Write(varBase + 6, 0x12);     // High byte of 0x1234
    _bus->Write(varBase + 7, 0xEF);     // Low byte of 0xBEEF
    _bus->Write(varBase + 8, 0xBE);     // High byte of 0xBEEF
    
    // Verify variable storage
    VerifyWRamValue(varBase, 0x01, "WRam variable boolean true");
    VerifyWRamValue(varBase + 1, 0x00, "WRam variable boolean false");
    VerifyWRamValue(varBase + 2, 0xFF, "WRam variable byte max");
    VerifyWRamValue(varBase + 3, 0x80, "WRam variable signed byte -128");
    VerifyWRamValue(varBase + 4, 0x7F, "WRam variable signed byte 127");
    VerifyWRamValue(varBase + 5, 0x34, "WRam variable 16-bit low byte");
    VerifyWRamValue(varBase + 6, 0x12, "WRam variable 16-bit high byte");
    VerifyWRamValue(varBase + 7, 0xEF, "WRam variable 16-bit low byte 2");
    VerifyWRamValue(varBase + 8, 0xBE, "WRam variable 16-bit high byte 2");
    
    LOG("    ✓ WRam variable storage test passed");
}

void WRamTest::TestWRamConstants()
{
    LOG("    Testing WRam constants...");
    
    // Test that WRam constants are correct
    if (AddressConstants::StartWRamAddress != 0xC000)
    {
        throw std::runtime_error("WRam StartWRamAddress constant incorrect");
    }
    if (AddressConstants::EndWRamAddress != 0xCFFF)
    {
        throw std::runtime_error("WRam EndWRamAddress constant incorrect");
    }
    
    // Test WRam size calculation
    word expectedSize = AddressConstants::EndWRamAddress - AddressConstants::StartWRamAddress + 1;
    if (expectedSize != 0x1000) // 4KB
    {
        throw std::runtime_error("WRam size calculation incorrect");
    }
    
    LOG("    ✓ WRam constants test passed");
}

void WRamTest::VerifyWRamValue(word address, byte expected, const std::string& testName)
{
    byte actual = _bus->Read(address);
    if (actual == expected) {
        LOG("    ✓ " + testName + " - expected 0x" + std::to_string(expected) + ", got 0x" + std::to_string(actual));
    } else {
        LOG("    ✗ " + testName + " - expected 0x" + std::to_string(expected) + ", got 0x" + std::to_string(actual));
        throw std::runtime_error(testName + " test failed");
    }
}

void WRamTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    for (size_t i = 0; i < program.size() && i < _cartridgeData.size(); i++)
    {
        _cartridgeData[i] = program[i];
    }
}

void WRamTest::ExecuteInstructions(int count)
{
    for (int i = 0; i < count; i++)
    {
        _cpu->Update();
    }
}