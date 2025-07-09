#include "SimpleCpuTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>
#include "Core/Logger.h"

SimpleCpuTest::SimpleCpuTest() : BaseTest("Simple CPU Test")
{
}

void SimpleCpuTest::Setup()
{
    LOG("  Setting up simple CPU test...");
    
    // Create minimal test setup
    _bootRomData = std::vector<byte>(256, 0x00);
    _cartridgeData = std::vector<byte>(32768, 0x00);
    
    // Set up minimal valid ROM header
    _cartridgeData[0x147] = 0x00;  // Cartridge type: ROM only
    _cartridgeData[0x148] = 0x00;  // ROM size: 32KB
    _cartridgeData[0x149] = 0x00;  // RAM size: None
    
    LOG("  Creating memory components...");
    
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
    
    LOG("  Creating bus...");
    
    // Create bus and CPU
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    
    LOG("  Creating CPU...");
    
    _cpu = std::make_unique<Cpu>(_bus.get());
    
    LOG("  Setup complete!");
}

void SimpleCpuTest::Run()
{
    LOG("  Testing basic CPU functionality...");
    
    // Test that we can write to memory
    _bus->Write(0xC000, 0x42);
    byte value = _bus->Read(0xC000);
    
    if (value == 0x42) {
        LOG("  ✓ Memory read/write works");
    } else {
        LOG("  ✗ Memory read/write failed");
        throw std::runtime_error("Memory test failed");
    }
    
    // Test that CPU doesn't crash on single update
    LOG("  Testing CPU update...");
    try {
        _cpu->Update();
        LOG("  ✓ CPU update completed without crashing");
    } catch (const std::exception& e) {
        LOG("  ✗ CPU update threw exception: " << e.what());
        throw;
    }
    
    LOG("  Simple CPU test completed successfully!");
}