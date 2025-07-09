#include "BootRomDisableTest.h"

#include <cassert>
#include <iostream>

void BootRomDisableTest::Setup()
{
    // Create minimal test setup
    _bootRomData = std::vector<byte>(256, 0x00);  // 256 bytes of boot ROM
    
    // Create valid cartridge ROM with proper header
    _cartridgeData = std::vector<byte>(32768, 0xFF);  // 32KB cartridge ROM
    
    // Set up minimal valid ROM header
    _cartridgeData[0x147] = 0x00;  // Cartridge type: ROM only (no MBC)
    _cartridgeData[0x148] = 0x00;  // ROM size: 32KB (code 0)
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
}

void BootRomDisableTest::Run()
{
    // Test 1: Boot ROM should be enabled initially (0xFF50 = 0)
    assert(_bus->Read(AddressConstants::BootRomBank) == 0);
    std::cout << "  ✓ Boot ROM bank register initially 0" << std::endl;
    
    // Test 2: Reading from 0x0000 should return boot ROM data when enabled
    byte bootValue = _bus->Read(0x0000);
    assert(bootValue == 0x00);  // Boot ROM data
    std::cout << "  ✓ Reading 0x0000 returns boot ROM data when enabled" << std::endl;
    
    // Test 3: Write to 0xFF50 to disable boot ROM
    _bus->Write(AddressConstants::BootRomBank, 0x01);
    assert(_bus->Read(AddressConstants::BootRomBank) == 0x01);
    std::cout << "  ✓ Writing to 0xFF50 stores the value" << std::endl;
    
    // Test 4: Reading from 0x0000 should now return cartridge data
    byte cartridgeValue = _bus->Read(0x0000);
    assert(cartridgeValue == 0xFF);  // Cartridge data
    std::cout << "  ✓ Reading 0x0000 returns cartridge data after boot ROM disable" << std::endl;
    
    // Test 5: Boot ROM should remain disabled even after writing different values
    _bus->Write(AddressConstants::BootRomBank, 0x42);
    assert(_bus->Read(AddressConstants::BootRomBank) == 0x42);
    byte stillCartridgeValue = _bus->Read(0x0000);
    assert(stillCartridgeValue == 0xFF);  // Still cartridge data
    std::cout << "  ✓ Boot ROM remains disabled with different 0xFF50 values" << std::endl;
}