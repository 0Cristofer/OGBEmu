#include "WRamCgbTest.h"
#include "Core/Logger.h"
#include "Emulator/Memory/AddressConstants.h"
#include <stdexcept>

WRamCgbTest::WRamCgbTest() : BaseTest("WRamCgb Test")
{
}

void WRamCgbTest::Setup()
{
    LOG("  Setting up WRamCgb test...");
    
    // WRamCgb is automatically initialized in constructor
    
    LOG("  WRamCgb test setup complete!");
}

void WRamCgbTest::Run()
{
    LOG("  Testing WRamCgb functionality...");
    
    TestWRamCgbBasicReadWrite();
    TestWRamCgbAddressTranslation();
    TestWRamCgbBoundaryConditions();
    TestWRamCgbInvalidAddresses();
    TestWRamCgbDataIntegrity();
    TestWRamCgbSequentialAccess();
    TestWRamCgbRandomAccess();
    TestWRamCgbConstants();
    
    LOG("  WRamCgb test completed successfully!");
}

void WRamCgbTest::TestWRamCgbBasicReadWrite()
{
    LOG("    Testing basic WRamCgb read/write...");
    
    // Test writing and reading from different addresses
    _wRamCgb.Write(AddressConstants::StartWRamCgbAddress, 0x42);
    byte value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress);
    if (value != 0x42) {
        throw std::runtime_error("WRamCgb basic write/read failed at start address");
    }
    LOG("    ✓ Basic WRamCgb write/read at " << std::hex << AddressConstants::StartWRamCgbAddress << " - expected 0x42, got 0x" << static_cast<int>(value));
    
    _wRamCgb.Write(AddressConstants::StartWRamCgbAddress + 1, 0x85);
    value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress + 1);
    if (value != 0x85) {
        throw std::runtime_error("WRamCgb basic write/read failed at start+1 address");
    }
    LOG("    ✓ Basic WRamCgb write/read at " << std::hex << (AddressConstants::StartWRamCgbAddress + 1) << " - expected 0x85, got 0x" << static_cast<int>(value));
    
    // Verify previous value is preserved
    value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress);
    if (value != 0x42) {
        throw std::runtime_error("WRamCgb value preservation failed");
    }
    LOG("    ✓ Previous WRamCgb value preservation - expected 0x42, got 0x" << static_cast<int>(value));
    
    // Test middle address
    _wRamCgb.Write(AddressConstants::StartWRamCgbAddress + 0x800, 0xAA);
    value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress + 0x800);
    if (value != 0xAA) {
        throw std::runtime_error("WRamCgb basic write/read failed at middle address");
    }
    LOG("    ✓ Basic WRamCgb write/read at " << std::hex << (AddressConstants::StartWRamCgbAddress + 0x800) << " - expected 0xAA, got 0x" << static_cast<int>(value));
    
    // Test end address
    _wRamCgb.Write(AddressConstants::EndWRamCgbAddress, 0x33);
    value = _wRamCgb.Read(AddressConstants::EndWRamCgbAddress);
    if (value != 0x33) {
        throw std::runtime_error("WRamCgb basic write/read failed at end address");
    }
    LOG("    ✓ Basic WRamCgb write/read at " << std::hex << AddressConstants::EndWRamCgbAddress << " - expected 0x33, got 0x" << static_cast<int>(value));
    
    LOG("    ✓ Basic WRamCgb read/write test passed");
}

void WRamCgbTest::TestWRamCgbAddressTranslation()
{
    LOG("    Testing WRamCgb address translation...");
    
    // Test that address translation works correctly
    _wRamCgb.Write(AddressConstants::StartWRamCgbAddress, 0x17);
    byte value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress);
    if (value != 0x17) {
        throw std::runtime_error("WRamCgb address translation failed at start");
    }
    LOG("    ✓ WRamCgb address translation " << std::hex << AddressConstants::StartWRamCgbAddress << " - expected 0x17, got 0x" << static_cast<int>(value));
    
    _wRamCgb.Write(AddressConstants::StartWRamCgbAddress + 0x800, 0x34);
    value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress + 0x800);
    if (value != 0x34) {
        throw std::runtime_error("WRamCgb address translation failed at middle");
    }
    LOG("    ✓ WRamCgb address translation " << std::hex << (AddressConstants::StartWRamCgbAddress + 0x800) << " - expected 0x34, got 0x" << static_cast<int>(value));
    
    _wRamCgb.Write(AddressConstants::EndWRamCgbAddress, 0x51);
    value = _wRamCgb.Read(AddressConstants::EndWRamCgbAddress);
    if (value != 0x51) {
        throw std::runtime_error("WRamCgb address translation failed at end");
    }
    LOG("    ✓ WRamCgb address translation " << std::hex << AddressConstants::EndWRamCgbAddress << " - expected 0x51, got 0x" << static_cast<int>(value));
    
    // Test address isolation
    value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress);
    if (value != 0x17) {
        throw std::runtime_error("WRamCgb address isolation failed");
    }
    LOG("    ✓ WRamCgb address isolation " << std::hex << AddressConstants::StartWRamCgbAddress << " - expected 0x17, got 0x" << static_cast<int>(value));
    
    value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress + 0x800);
    if (value != 0x34) {
        throw std::runtime_error("WRamCgb address isolation failed at middle");
    }
    LOG("    ✓ WRamCgb address isolation " << std::hex << (AddressConstants::StartWRamCgbAddress + 0x800) << " - expected 0x34, got 0x" << static_cast<int>(value));
    
    LOG("    ✓ WRamCgb address translation test passed");
}

void WRamCgbTest::TestWRamCgbBoundaryConditions()
{
    LOG("    Testing WRamCgb boundary conditions...");
    
    // Test start boundary
    _wRamCgb.Write(AddressConstants::StartWRamCgbAddress, 0x68);
    byte value = _wRamCgb.Read(AddressConstants::StartWRamCgbAddress);
    if (value != 0x68) {
        throw std::runtime_error("WRamCgb start boundary test failed");
    }
    LOG("    ✓ WRamCgb start boundary " << std::hex << AddressConstants::StartWRamCgbAddress << " - expected 0x68, got 0x" << static_cast<int>(value));
    
    // Test end boundary
    _wRamCgb.Write(AddressConstants::EndWRamCgbAddress, 0x85);
    value = _wRamCgb.Read(AddressConstants::EndWRamCgbAddress);
    if (value != 0x85) {
        throw std::runtime_error("WRamCgb end boundary test failed");
    }
    LOG("    ✓ WRamCgb end boundary " << std::hex << AddressConstants::EndWRamCgbAddress << " - expected 0x85, got 0x" << static_cast<int>(value));
    
    // Test addresses just outside boundaries - these should not affect WRamCgb
    word belowStart = AddressConstants::StartWRamCgbAddress - 1;
    word aboveEnd = AddressConstants::EndWRamCgbAddress + 1;
    
    // Reading from addresses outside WRamCgb should return 0 (handled by WRamCgb itself)
    value = _wRamCgb.Read(belowStart);
    if (value != 0x00) {
        throw std::runtime_error("WRamCgb should return 0 for address below start");
    }
    LOG("    ✓ WRamCgb isolation from " << std::hex << belowStart << " - expected 0x68, got 0x" << static_cast<int>(_wRamCgb.Read(AddressConstants::StartWRamCgbAddress)));
    
    value = _wRamCgb.Read(aboveEnd);
    if (value != 0x00) {
        throw std::runtime_error("WRamCgb should return 0 for address above end");
    }
    LOG("    ✓ WRamCgb isolation from " << std::hex << aboveEnd << " - expected 0x85, got 0x" << static_cast<int>(_wRamCgb.Read(AddressConstants::EndWRamCgbAddress)));
    
    LOG("    ✓ WRamCgb boundary conditions test passed");
}

void WRamCgbTest::TestWRamCgbInvalidAddresses()
{
    LOG("    Testing WRamCgb invalid address handling...");
    
    // Test addresses well outside the valid range
    word invalidLow = 0x1000;
    word invalidHigh = 0xF000;
    
    // These should return 0 and not crash
    byte value = _wRamCgb.Read(invalidLow);
    if (value != 0x00) {
        throw std::runtime_error("WRamCgb should return 0 for invalid low address");
    }
    LOG("    ✓ WRamCgb invalid low address " << std::hex << invalidLow << " - expected 0x0, got 0x" << static_cast<int>(value));
    
    value = _wRamCgb.Read(invalidHigh);
    if (value != 0x00) {
        throw std::runtime_error("WRamCgb should return 0 for invalid high address");
    }
    LOG("    ✓ WRamCgb invalid high address " << std::hex << invalidHigh << " - expected 0x0, got 0x" << static_cast<int>(value));
    
    // Test writing to invalid addresses - should not crash
    _wRamCgb.Write(invalidLow, 0x42);
    _wRamCgb.Write(invalidHigh, 0x42);
    
    LOG("    ✓ WRamCgb invalid address handling test passed");
}

void WRamCgbTest::TestWRamCgbDataIntegrity()
{
    LOG("    Testing WRamCgb data integrity...");
    
    // Write a pattern and verify it's preserved
    const byte pattern[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    const word baseAddress = AddressConstants::StartWRamCgbAddress + 0x100;
    
    // Write pattern
    for (int i = 0; i < 8; i++) {
        _wRamCgb.Write(baseAddress + i, pattern[i]);
    }
    
    // Read and verify pattern
    for (int i = 0; i < 8; i++) {
        byte value = _wRamCgb.Read(baseAddress + i);
        if (value != pattern[i]) {
            throw std::runtime_error("WRamCgb data integrity failed at offset " + std::to_string(i));
        }
        LOG("    ✓ WRamCgb data integrity at offset " << i << " - expected 0x" << static_cast<int>(pattern[i]) << ", got 0x" << static_cast<int>(value));
    }
    
    LOG("    ✓ WRamCgb data integrity test passed");
}

void WRamCgbTest::TestWRamCgbSequentialAccess()
{
    LOG("    Testing WRamCgb sequential access...");
    
    // Write sequential values
    const word startAddr = AddressConstants::StartWRamCgbAddress + 0x200;
    const int count = 16;
    
    for (int i = 0; i < count; i++) {
        _wRamCgb.Write(startAddr + i, static_cast<byte>(i + 0x50));
    }
    
    // Read and verify sequential values
    for (int i = 0; i < count; i++) {
        byte expected = static_cast<byte>(i + 0x50);
        byte value = _wRamCgb.Read(startAddr + i);
        if (value != expected) {
            throw std::runtime_error("WRamCgb sequential access failed at offset " + std::to_string(i));
        }
        LOG("    ✓ WRamCgb sequential access at offset " << i << " - expected 0x" << static_cast<int>(expected) << ", got 0x" << static_cast<int>(value));
    }
    
    LOG("    ✓ WRamCgb sequential access test passed");
}

void WRamCgbTest::TestWRamCgbRandomAccess()
{
    LOG("    Testing WRamCgb random access...");
    
    // Test random addresses within the valid range
    struct TestCase {
        word address;
        byte value;
    };
    
    const TestCase testCases[] = {
        {AddressConstants::StartWRamCgbAddress + 0x123, 0xAB},
        {AddressConstants::StartWRamCgbAddress + 0x456, 0xCD},
        {AddressConstants::StartWRamCgbAddress + 0x789, 0xEF},
        {AddressConstants::StartWRamCgbAddress + 0xABC, 0x12},
        {AddressConstants::EndWRamCgbAddress - 0x100, 0x34}
    };
    
    // Write random values
    for (const auto& testCase : testCases) {
        _wRamCgb.Write(testCase.address, testCase.value);
    }
    
    // Read and verify random values
    for (const auto& testCase : testCases) {
        byte value = _wRamCgb.Read(testCase.address);
        if (value != testCase.value) {
            throw std::runtime_error("WRamCgb random access failed at address " + std::to_string(testCase.address));
        }
        LOG("    ✓ WRamCgb random access at " << std::hex << testCase.address << " - expected 0x" << static_cast<int>(testCase.value) << ", got 0x" << static_cast<int>(value));
    }
    
    LOG("    ✓ WRamCgb random access test passed");
}

void WRamCgbTest::TestWRamCgbConstants()
{
    LOG("    Testing WRamCgb constants...");
    
    // Verify the address constants are correct
    if (AddressConstants::StartWRamCgbAddress != 0xD000) {
        throw std::runtime_error("WRamCgb start address constant incorrect");
    }
    
    if (AddressConstants::EndWRamCgbAddress != 0xDFFF) {
        throw std::runtime_error("WRamCgb end address constant incorrect");
    }
    
    // Verify the range size is correct (4KB)
    word expectedSize = AddressConstants::EndWRamCgbAddress - AddressConstants::StartWRamCgbAddress + 1;
    if (expectedSize != 0x1000) {
        throw std::runtime_error("WRamCgb size calculation incorrect");
    }
    
    LOG("    ✓ WRamCgb constants test passed");
}