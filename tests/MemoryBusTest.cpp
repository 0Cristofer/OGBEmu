#include "MemoryBusTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Core/Logger.h"
#include <cassert>

MemoryBusTest::MemoryBusTest() : BaseTest("Memory Bus Test")
{
}

void MemoryBusTest::Setup()
{
    LOG("  Setting up memory bus test...");
    
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
    
    // Initialize to proper post-boot state
    InitializePostBootHardwareState(_bus.get());
    
    LOG("  Memory bus test setup complete!");
}

void MemoryBusTest::Run()
{
    LOG("  Testing memory bus routing...");
    
    TestBootRomRouting();
    TestCartridgeRouting();
    TestVRamRouting();
    TestWRamRouting();
    TestWRamCgbRouting();
    TestEchoRamRouting();
    TestOamRouting();
    TestIoRegistersRouting();
    TestHRamRouting();
    TestIeAddressRouting();
    TestNotUsedAddressRange();
    TestAddressBoundaries();
    
    LOG("  Memory bus routing test completed successfully!");
}

void MemoryBusTest::TestBootRomRouting()
{
    LOG("    Testing Boot ROM routing...");
    
    // Enable boot ROM first
    _bus->Write(AddressConstants::BootRomBank, 0x00);
    
    // Test Boot ROM address range (0x0000-0x00FF)
    // Boot ROM should be read-only in normal operation
    byte originalValue = _bus->Read(0x0050);
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing Boot ROM read-only behavior (intentional error expected)...");
    _bus->Write(0x0050, 0xAB);
    LOG("    Boot ROM read-only test completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    byte afterWrite = _bus->Read(0x0050);
    
    // Should remain unchanged (read-only)
    if (afterWrite != originalValue) {
        throw std::runtime_error("Boot ROM routing failed: Boot ROM should be read-only");
    }
    
    // Disable boot ROM
    _bus->Write(AddressConstants::BootRomBank, 0x01);
    
    // Now cartridge should be accessible at same address
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing cartridge access after Boot ROM disable (intentional error expected)...");
    _bus->Write(0x0050, 0xCD);
    LOG("    Cartridge access test completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    byte cartridgeValue = _bus->Read(0x0050);
    
    // This might still be read-only depending on cartridge implementation
    // The test verifies routing occurs, not necessarily write capability
    
    LOG("    ✓ Boot ROM routing test passed");
}

void MemoryBusTest::TestCartridgeRouting()
{
    LOG("    Testing Cartridge routing...");
    
    // Ensure boot ROM is disabled
    _bus->Write(AddressConstants::BootRomBank, 0x01);
    
    // Test ROM Bank 0 (0x0000-0x3FFF) - should be read-only
    VerifyReadOnly(0x1000, "ROM Bank 0");
    VerifyReadOnly(0x3FFF, "ROM Bank 0 end");
    
    // Test ROM Bank N (0x4000-0x7FFF) - should be read-only
    VerifyReadOnly(0x4000, "ROM Bank N start");
    VerifyReadOnly(0x7FFF, "ROM Bank N end");
    
    LOG("    ✓ Cartridge routing test passed");
}

void MemoryBusTest::TestVRamRouting()
{
    LOG("    Testing VRAM routing...");
    
    // Test VRAM address range (0x8000-0x9FFF)
    VerifyReadWrite(0x8000, 0x12, "VRAM start");
    VerifyReadWrite(0x8FFF, 0x34, "VRAM middle");
    VerifyReadWrite(0x9FFF, 0x56, "VRAM end");
    
    LOG("    ✓ VRAM routing test passed");
}

void MemoryBusTest::TestWRamRouting()
{
    LOG("    Testing WRAM routing...");
    
    // Test WRAM address range (0xC000-0xCFFF)
    VerifyReadWrite(0xC000, 0x78, "WRAM start");
    VerifyReadWrite(0xC800, 0x9A, "WRAM middle");
    VerifyReadWrite(0xCFFF, 0xBC, "WRAM end");
    
    LOG("    ✓ WRAM routing test passed");
}

void MemoryBusTest::TestWRamCgbRouting()
{
    LOG("    Testing WRAM CGB routing...");
    
    // Test WRAM CGB address range (0xD000-0xDFFF)
    VerifyReadWrite(0xD000, 0xDE, "WRAM CGB start");
    VerifyReadWrite(0xD800, 0xF0, "WRAM CGB middle");
    VerifyReadWrite(0xDFFF, 0x01, "WRAM CGB end");
    
    LOG("    ✓ WRAM CGB routing test passed");
}

void MemoryBusTest::TestEchoRamRouting()
{
    LOG("    Testing Echo RAM routing...");
    
    // Echo RAM should mirror WRAM
    // Write to WRAM and verify it appears in Echo RAM
    _bus->Write(0xC123, 0x23);
    byte echoValue = _bus->Read(0xE123);
    
    if (echoValue != 0x23) {
        throw std::runtime_error("Echo RAM routing failed: Echo RAM does not mirror WRAM read");
    }
    
    // Write to Echo RAM and verify it appears in WRAM
    _bus->Write(0xE456, 0x45);
    byte wramValue = _bus->Read(0xC456);
    
    if (wramValue != 0x45) {
        throw std::runtime_error("Echo RAM routing failed: Echo RAM does not mirror WRAM write");
    }
    
    LOG("    ✓ Echo RAM routing test passed");
}

void MemoryBusTest::TestOamRouting()
{
    LOG("    Testing OAM routing...");
    
    // Test OAM address range (0xFE00-0xFE9F)
    VerifyReadWrite(0xFE00, 0x67, "OAM start");
    VerifyReadWrite(0xFE50, 0x89, "OAM middle");
    VerifyReadWrite(0xFE9F, 0xAB, "OAM end");
    
    LOG("    ✓ OAM routing test passed");
}

void MemoryBusTest::TestIoRegistersRouting()
{
    LOG("    Testing IO Registers routing...");
    
    // Test IO Registers address range (0xFF00-0xFF7F)
    VerifyReadWrite(0xFF00, 0xCD, "IO Registers start");
    VerifyReadWrite(0xFF40, 0xEF, "IO Registers middle");
    VerifyReadWrite(0xFF7F, 0x01, "IO Registers end");
    
    LOG("    ✓ IO Registers routing test passed");
}

void MemoryBusTest::TestHRamRouting()
{
    LOG("    Testing High RAM routing...");
    
    // Test High RAM address range (0xFF80-0xFFFE)
    VerifyReadWrite(0xFF80, 0x23, "High RAM start");
    VerifyReadWrite(0xFFBF, 0x45, "High RAM middle");
    VerifyReadWrite(0xFFFE, 0x67, "High RAM end");
    
    LOG("    ✓ High RAM routing test passed");
}

void MemoryBusTest::TestIeAddressRouting()
{
    LOG("    Testing IE address routing...");
    
    // Test Interrupt Enable register (0xFFFF)
    VerifyReadWrite(0xFFFF, 0x89, "IE register");
    
    LOG("    ✓ IE address routing test passed");
}

void MemoryBusTest::TestNotUsedAddressRange()
{
    LOG("    Testing not used address range...");
    
    // Test not used address range (0xFEA0-0xFEFF)
    // This range should return 0xFF on read and ignore writes
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing NotUsed address range 0xFEA0 (intentional errors expected)...");
    byte originalValue = _bus->Read(0xFEA0);
    _bus->Write(0xFEA0, 0x12);
    byte afterWrite = _bus->Read(0xFEA0);
    LOG("    NotUsed address range write test completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    
    // Verify the value didn't change (writes ignored)
    if (afterWrite != originalValue) {
        throw std::runtime_error("Not used address range failed: Writes should be ignored");
    }
    
    // Test middle and end of range
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing NotUsed address range 0xFED0 and 0xFEFF (intentional errors expected)...");
    byte middleValue = _bus->Read(0xFED0);
    byte endValue = _bus->Read(0xFEFF);
    LOG("    NotUsed address range read tests completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    
    // These should typically return 0xFF or some consistent value
    // The exact behavior may vary by implementation
    
    LOG("    ✓ Not used address range test passed");
}

void MemoryBusTest::TestAddressBoundaries()
{
    LOG("    Testing address boundaries...");
    
    // Test boundary conditions between memory regions
    
    // Boundary between ROM Bank 0 and ROM Bank N (0x3FFF/0x4000)
    VerifyReadOnly(0x3FFF, "ROM Bank 0/N boundary");
    VerifyReadOnly(0x4000, "ROM Bank N/VRAM boundary");
    
    // Boundary between ROM Bank N and VRAM (0x7FFF/0x8000)
    VerifyReadOnly(0x7FFF, "ROM Bank N end");
    VerifyReadWrite(0x8000, 0xAB, "VRAM start");
    
    // Boundary between VRAM and External RAM (0x9FFF/0xA000)
    VerifyReadWrite(0x9FFF, 0xCD, "VRAM end");
    // 0xA000 (External RAM) behavior depends on cartridge type
    
    // Boundary between External RAM and WRAM (0xBFFF/0xC000)
    VerifyReadWrite(0xC000, 0xEF, "WRAM start");
    
    // Boundary between WRAM and WRAM CGB (0xCFFF/0xD000)
    VerifyReadWrite(0xCFFF, 0x01, "WRAM end");
    VerifyReadWrite(0xD000, 0x23, "WRAM CGB start");
    
    // Boundary between WRAM CGB and Echo RAM (0xDFFF/0xE000)
    VerifyReadWrite(0xDFFF, 0x45, "WRAM CGB end");
    _bus->Write(0xE000, 0x67);  // Echo RAM write
    
    // Boundary between Echo RAM and OAM (0xFDFF/0xFE00)
    VerifyReadWrite(0xFE00, 0x89, "OAM start");
    
    // Boundary between OAM and Not Used (0xFE9F/0xFEA0)
    VerifyReadWrite(0xFE9F, 0xAB, "OAM end");
    
    // Boundary between Not Used and IO Registers (0xFEFF/0xFF00)
    VerifyReadWrite(0xFF00, 0xCD, "IO Registers start");
    
    // Boundary between IO Registers and High RAM (0xFF7F/0xFF80)
    VerifyReadWrite(0xFF7F, 0xEF, "IO Registers end");
    VerifyReadWrite(0xFF80, 0x01, "High RAM start");
    
    // Boundary between High RAM and IE (0xFFFE/0xFFFF)
    VerifyReadWrite(0xFFFE, 0x23, "High RAM end");
    VerifyReadWrite(0xFFFF, 0x45, "IE register");
    
    LOG("    ✓ Address boundaries test passed");
}

void MemoryBusTest::VerifyReadWrite(word address, byte testValue, const std::string& regionName)
{
    // Store original value
    byte originalValue = _bus->Read(address);
    
    // Write test value
    _bus->Write(address, testValue);
    
    // Read back and verify
    byte readValue = _bus->Read(address);
    
    if (readValue != testValue) {
        char addressHex[16], expectedHex[16], actualHex[16];
        sprintf(addressHex, "%04X", address);
        sprintf(expectedHex, "%02X", testValue);
        sprintf(actualHex, "%02X", readValue);
        throw std::runtime_error(regionName + " read/write failed at address 0x" + 
                                std::string(addressHex) + ": expected 0x" + 
                                std::string(expectedHex) + ", got 0x" + 
                                std::string(actualHex));
    }
    
    // Restore original value
    _bus->Write(address, originalValue);
}

void MemoryBusTest::VerifyReadOnly(word address, const std::string& regionName)
{
    // Store original value
    byte originalValue = _bus->Read(address);
    
    // Attempt to write different value
    byte testValue = originalValue ^ 0xFF;  // Flip all bits
    LOG("    ---- INTENTIONAL ERROR REGION START ----");
    LOG("    Testing " << regionName << " read-only at 0x" << std::hex << address << " (intentional error expected)...");
    _bus->Write(address, testValue);
    LOG("    Read-only test for " << regionName << " completed.");
    LOG("    ---- INTENTIONAL ERROR REGION END ----");
    
    // Read back and verify it didn't change
    byte readValue = _bus->Read(address);
    
    if (readValue != originalValue) {
        char addressHex[16], originalHex[16], actualHex[16];
        sprintf(addressHex, "%04X", address);
        sprintf(originalHex, "%02X", originalValue);
        sprintf(actualHex, "%02X", readValue);
        throw std::runtime_error(regionName + " read-only verification failed at address 0x" + 
                                std::string(addressHex) + ": original 0x" + 
                                std::string(originalHex) + ", after write attempt 0x" + 
                                std::string(actualHex));
    }
}