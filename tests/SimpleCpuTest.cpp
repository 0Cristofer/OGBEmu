#include "SimpleCpuTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include <cassert>
#include <iostream>

SimpleCpuTest::SimpleCpuTest() : BaseTest("Simple CPU Test")
{
}

void SimpleCpuTest::Setup()
{
    std::cout << "  Setting up simple CPU test..." << std::endl;
    
    // Create minimal test setup
    _bootRomData = std::vector<byte>(256, 0x00);
    _cartridgeData = std::vector<byte>(32768, 0x00);
    
    // Set up minimal valid ROM header
    _cartridgeData[0x147] = 0x00;  // Cartridge type: ROM only
    _cartridgeData[0x148] = 0x00;  // ROM size: 32KB
    _cartridgeData[0x149] = 0x00;  // RAM size: None
    
    std::cout << "  Creating memory components..." << std::endl;
    
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
    
    std::cout << "  Creating bus..." << std::endl;
    
    // Create bus and CPU
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    
    std::cout << "  Creating CPU..." << std::endl;
    
    _cpu = std::make_unique<Cpu>(_bus.get());
    
    std::cout << "  Setup complete!" << std::endl;
}

void SimpleCpuTest::Run()
{
    std::cout << "  Testing basic CPU functionality..." << std::endl;
    
    // Test that we can write to memory
    _bus->Write(0xC000, 0x42);
    byte value = _bus->Read(0xC000);
    
    if (value == 0x42) {
        std::cout << "  ✓ Memory read/write works" << std::endl;
    } else {
        std::cout << "  ✗ Memory read/write failed" << std::endl;
        throw std::runtime_error("Memory test failed");
    }
    
    // Test that CPU doesn't crash on single update
    std::cout << "  Testing CPU update..." << std::endl;
    try {
        _cpu->Update();
        std::cout << "  ✓ CPU update completed without crashing" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ CPU update threw exception: " << e.what() << std::endl;
        throw;
    }
    
    std::cout << "  Simple CPU test completed successfully!" << std::endl;
}