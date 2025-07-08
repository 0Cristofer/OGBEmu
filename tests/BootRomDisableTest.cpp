#include <cassert>
#include <iostream>

#include "../OGBEmu/src/Emulator/Memory/Bus.h"
#include "../OGBEmu/src/Emulator/Memory/BootRom.h"
#include "../OGBEmu/src/Emulator/Memory/Cartridge.h"
#include "../OGBEmu/src/Emulator/Memory/VRam.h"
#include "../OGBEmu/src/Emulator/Memory/WRam.h"
#include "../OGBEmu/src/Emulator/Memory/WRamCgb.h"
#include "../OGBEmu/src/Emulator/Memory/EchoRam.h"
#include "../OGBEmu/src/Emulator/Memory/Oam.h"
#include "../OGBEmu/src/Emulator/Memory/IoRegisters.h"
#include "../OGBEmu/src/Emulator/Memory/HRam.h"
#include "../OGBEmu/src/Emulator/Memory/AddressConstants.h"

int main()
{
    std::cout << "Testing Boot ROM disable mechanism..." << std::endl;
    
    // Create minimal test setup
    std::vector<byte> bootRomData(256, 0x00);  // 256 bytes of boot ROM
    
    // Create valid cartridge ROM with proper header
    std::vector<byte> cartridgeData(32768, 0xFF);  // 32KB cartridge ROM
    
    // Set up minimal valid ROM header
    cartridgeData[0x147] = 0x00;  // Cartridge type: ROM only (no MBC)
    cartridgeData[0x148] = 0x00;  // ROM size: 32KB (code 0)
    cartridgeData[0x149] = 0x00;  // RAM size: None
    
    // Create memory components
    BootRom bootRom(bootRomData);
    Cartridge cartridge(cartridgeData);
    VRam vRam;
    WRam wRam;
    WRamCgb wRamCgb;
    EchoRam echoRam;
    Oam oam;
    IoRegisters ioRegisters;
    HRam hRam;
    
    // Create bus
    Bus bus(&bootRom, &cartridge, &vRam, &wRam, &wRamCgb, &echoRam, &oam, &ioRegisters, &hRam);
    
    // Test 1: Boot ROM should be enabled initially (0xFF50 = 0)
    assert(bus.Read(AddressConstants::BootRomBank) == 0);
    std::cout << "✓ Boot ROM bank register initially 0" << std::endl;
    
    // Test 2: Reading from 0x0000 should return boot ROM data when enabled
    byte bootValue = bus.Read(0x0000);
    assert(bootValue == 0x00);  // Boot ROM data
    std::cout << "✓ Reading 0x0000 returns boot ROM data when enabled" << std::endl;
    
    // Test 3: Write to 0xFF50 to disable boot ROM
    bus.Write(AddressConstants::BootRomBank, 0x01);
    assert(bus.Read(AddressConstants::BootRomBank) == 0x01);
    std::cout << "✓ Writing to 0xFF50 stores the value" << std::endl;
    
    // Test 4: Reading from 0x0000 should now return cartridge data
    byte cartridgeValue = bus.Read(0x0000);
    assert(cartridgeValue == 0xFF);  // Cartridge data
    std::cout << "✓ Reading 0x0000 returns cartridge data after boot ROM disable" << std::endl;
    
    // Test 5: Boot ROM should remain disabled even after writing different values
    bus.Write(AddressConstants::BootRomBank, 0x42);
    assert(bus.Read(AddressConstants::BootRomBank) == 0x42);
    byte stillCartridgeValue = bus.Read(0x0000);
    assert(stillCartridgeValue == 0xFF);  // Still cartridge data
    std::cout << "✓ Boot ROM remains disabled with different 0xFF50 values" << std::endl;
    
    std::cout << "\nAll Boot ROM disable tests passed! ✓" << std::endl;
    return 0;
}