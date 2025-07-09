#include "BaseTest.h"

#include <iostream>
#include <exception>
#include "Core/Logger.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/AddressConstants.h"

BaseTest::BaseTest(const std::string& testName) : _testName(testName)
{
}

void BaseTest::Execute()
{
    LOG("Running test: " << _testName);
    
    try
    {
        Setup();
        Run();
        LOG("✓ " << _testName << " PASSED");
    }
    catch (const std::exception& e)
    {
        LOG("✗ " << _testName << " FAILED: " << e.what());
        throw;
    }
    catch (...)
    {
        LOG("✗ " << _testName << " FAILED: Unknown exception");
        throw;
    }
}

const std::string& BaseTest::GetTestName() const
{
    return _testName;
}

void BaseTest::InitializePostBootHardwareState(Bus* bus)
{
    // Initialize hardware registers to post-boot state
    // Based on https://gbdev.io/pandocs/Power_Up_Sequence.html
    // Values for DMG (original Game Boy)
    
    // LCD Control Register
    bus->Write(0xFF40, 0x91);  // LCDC = $91
    
    // Background Palette
    bus->Write(0xFF47, 0xFC);  // BGP = $FC
    
    // Audio Master Control
    bus->Write(0xFF26, 0xF1);  // NR52 = $F1
    
    // Timer Control
    bus->Write(0xFF07, 0xF8);  // TAC = $F8
    
    // Interrupt Flag
    bus->Write(0xFF0F, 0xE1);  // IF = $E1
    
    // Disable boot ROM
    bus->Write(AddressConstants::BootRomBank, 0x01);  // Boot ROM disabled
}