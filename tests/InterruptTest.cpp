#include "InterruptTest.h"
#include "Emulator/Memory/AddressConstants.h"
#include "Core/Logger.h"
#include <cassert>

InterruptTest::InterruptTest() : BaseTest("Interrupt System Test")
{
}

void InterruptTest::Setup()
{
    LOG("  Setting up interrupt system test...");
    
    // Create minimal boot ROM
    _bootRomData = std::vector<byte>(256, 0x00);
    
    // Create minimal cartridge with interrupt handlers pre-installed
    _cartridgeData = std::vector<byte>(0x8000, 0x00);
    _cartridgeData[0x147] = 0x00;  // No MBC
    _cartridgeData[0x148] = 0x00;  // ROM size: 32KB
    _cartridgeData[0x149] = 0x00;  // RAM size: None
    
    // Pre-install interrupt handlers in the cartridge
    // Use RET (0xC9) instead of HALT so the handlers return to main program
    _cartridgeData[AddressConstants::VBlankHandlerAddress] = 0xC9;   // RET at VBlank handler
    _cartridgeData[AddressConstants::LcdHandlerAddress] = 0xC9;      // RET at LCD handler  
    _cartridgeData[AddressConstants::TimerHandlerAddress] = 0xC9;    // RET at Timer handler
    _cartridgeData[AddressConstants::SerialHandlerAddress] = 0xC9;   // RET at Serial handler
    _cartridgeData[AddressConstants::JoypadHandlerAddress] = 0xC9;   // RET at Joypad handler
    
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
    
    // Create CPU
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Initialize to proper post-boot state
    InitializePostBootHardwareState(_bus.get());
    
    LOG("  Interrupt system test setup complete!");
}

void InterruptTest::Run()
{
    LOG("  Testing interrupt system...");
    
    TestInterruptEnableRegister();
    TestInterruptFlagRegister();
    TestVBlankInterrupt();
    TestLcdInterrupt();
    TestTimerInterrupt();
    TestSerialInterrupt();
    TestJoypadInterrupt();
    TestInterruptPriority();
    TestInterruptMasterEnable();
    TestInterruptHandlerAddresses();
    
    LOG("  Interrupt system test completed successfully!");
}

void InterruptTest::TestInterruptEnableRegister()
{
    LOG("    Testing Interrupt Enable register...");
    
    // Test writing to IE register (0xFFFF)
    _bus->Write(0xFFFF, 0x1F);  // Enable all interrupts
    byte ieValue = _bus->Read(0xFFFF);
    
    if (ieValue != 0x1F) {
        throw std::runtime_error("IE register write/read failed: expected 0x1F, got 0x" + 
                                std::to_string(ieValue));
    }
    
    // Test individual interrupt enable bits
    _bus->Write(0xFFFF, 0x01);  // Only VBlank
    ieValue = _bus->Read(0xFFFF);
    if (ieValue != 0x01) {
        throw std::runtime_error("IE register VBlank bit failed");
    }
    
    _bus->Write(0xFFFF, 0x02);  // Only LCD
    ieValue = _bus->Read(0xFFFF);
    if (ieValue != 0x02) {
        throw std::runtime_error("IE register LCD bit failed");
    }
    
    _bus->Write(0xFFFF, 0x04);  // Only Timer
    ieValue = _bus->Read(0xFFFF);
    if (ieValue != 0x04) {
        throw std::runtime_error("IE register Timer bit failed");
    }
    
    _bus->Write(0xFFFF, 0x08);  // Only Serial
    ieValue = _bus->Read(0xFFFF);
    if (ieValue != 0x08) {
        throw std::runtime_error("IE register Serial bit failed");
    }
    
    _bus->Write(0xFFFF, 0x10);  // Only Joypad
    ieValue = _bus->Read(0xFFFF);
    if (ieValue != 0x10) {
        throw std::runtime_error("IE register Joypad bit failed");
    }
    
    LOG("    ✓ Interrupt Enable register test passed");
}

void InterruptTest::TestInterruptFlagRegister()
{
    LOG("    Testing Interrupt Flag register...");
    
    // Test writing to IF register (0xFF0F)
    _bus->Write(AddressConstants::InterruptFlag, 0x1F);  // Set all interrupt flags
    byte ifValue = _bus->Read(AddressConstants::InterruptFlag);
    
    if (ifValue != 0x1F) {
        throw std::runtime_error("IF register write/read failed: expected 0x1F, got 0x" + 
                                std::to_string(ifValue));
    }
    
    // Test clearing individual flags
    _bus->Write(AddressConstants::InterruptFlag, 0x1E);  // Clear VBlank
    ifValue = _bus->Read(AddressConstants::InterruptFlag);
    if (ifValue != 0x1E) {
        throw std::runtime_error("IF register VBlank clear failed");
    }
    
    _bus->Write(AddressConstants::InterruptFlag, 0x00);  // Clear all
    ifValue = _bus->Read(AddressConstants::InterruptFlag);
    if (ifValue != 0x00) {
        throw std::runtime_error("IF register clear all failed");
    }
    
    LOG("    ✓ Interrupt Flag register test passed");
}

void InterruptTest::TestVBlankInterrupt()
{
    LOG("    Testing VBlank interrupt handling...");
    
    // Create a simple program that enables interrupts and waits
    std::vector<byte> program = {
        0xFB,      // EI (enable interrupts)
        0x00,      // NOP
        0x76       // HALT
    };
    
    WriteProgramToCartridge(program);
    
    // Enable VBlank interrupt
    _bus->Write(0xFFFF, 0x01);  // IE: Enable VBlank
    
    // Clear any existing interrupt flags first
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Trigger VBlank interrupt manually (tests interrupt handling mechanism)
    TriggerInterrupt(0x01);  // Set VBlank flag
    
    // Verify the interrupt flag was set
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x01) == 0) {
        throw std::runtime_error("VBlank interrupt flag not set");
    }
    
    // Execute cycles to allow interrupt handling
    ExecuteInstructions(10);
    
    // Verify interrupt flag was cleared after handling (this proves the interrupt was processed)
    interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x01) != 0) {
        throw std::runtime_error("VBlank interrupt flag not cleared after handling - interrupt not processed");
    }
    
    // Additional verification: PC should not be at the original program location 
    // since interrupt handling should have changed execution flow
    word currentPc = _cpu->GetPC();
    if (currentPc == 0x100) {  // 0x100 is where the test program starts
        throw std::runtime_error("PC still at original location - interrupt handler may not have been called");
    }
    
    LOG("    ✓ VBlank interrupt handling test passed");
}

void InterruptTest::TestLcdInterrupt()
{
    LOG("    Testing LCD interrupt...");
    
    // Create a simple program that enables interrupts and waits
    std::vector<byte> program = {
        0xFB,      // EI (enable interrupts)
        0x00,      // NOP
        0x76       // HALT
    };
    
    WriteProgramToCartridge(program);
    
    // Enable LCD interrupt
    _bus->Write(0xFFFF, 0x02);  // IE: Enable LCD
    
    // Clear any existing interrupt flags first
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Trigger LCD interrupt
    TriggerInterrupt(0x02);  // Set LCD flag
    
    // Execute a few cycles to allow interrupt handling
    ExecuteInstructions(10);
    
    // Verify interrupt flag was cleared after handling
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x02) != 0) {
        throw std::runtime_error("LCD interrupt flag not cleared after handling");
    }
    
    LOG("    ✓ LCD interrupt test passed");
}

void InterruptTest::TestTimerInterrupt()
{
    LOG("    Testing Timer interrupt...");
    
    // Create a simple program that enables interrupts and waits
    std::vector<byte> program = {
        0xFB,      // EI (enable interrupts)
        0x00,      // NOP
        0x76       // HALT
    };
    
    WriteProgramToCartridge(program);
    
    // Enable Timer interrupt
    _bus->Write(0xFFFF, 0x04);  // IE: Enable Timer
    
    // Clear any existing interrupt flags first
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Trigger Timer interrupt
    TriggerInterrupt(0x04);  // Set Timer flag
    
    // Execute a few cycles to allow interrupt handling
    ExecuteInstructions(10);
    
    // Verify interrupt flag was cleared after handling
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x04) != 0) {
        throw std::runtime_error("Timer interrupt flag not cleared after handling");
    }
    
    LOG("    ✓ Timer interrupt test passed");
}

void InterruptTest::TestSerialInterrupt()
{
    LOG("    Testing Serial interrupt...");
    
    // Create a simple program that enables interrupts and waits
    std::vector<byte> program = {
        0xFB,      // EI (enable interrupts)
        0x00,      // NOP
        0x76       // HALT
    };
    
    WriteProgramToCartridge(program);
    
    // Enable Serial interrupt
    _bus->Write(0xFFFF, 0x08);  // IE: Enable Serial
    
    // Clear any existing interrupt flags first
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Trigger Serial interrupt
    TriggerInterrupt(0x08);  // Set Serial flag
    
    // Execute a few cycles to allow interrupt handling
    ExecuteInstructions(10);
    
    // Verify interrupt flag was cleared after handling
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x08) != 0) {
        throw std::runtime_error("Serial interrupt flag not cleared after handling");
    }
    
    LOG("    ✓ Serial interrupt test passed");
}

void InterruptTest::TestJoypadInterrupt()
{
    LOG("    Testing Joypad interrupt...");
    
    // Create a simple program that enables interrupts and waits
    std::vector<byte> program = {
        0xFB,      // EI (enable interrupts)
        0x00,      // NOP
        0x76       // HALT
    };
    
    WriteProgramToCartridge(program);
    
    // Enable Joypad interrupt
    _bus->Write(0xFFFF, 0x10);  // IE: Enable Joypad
    
    // Clear any existing interrupt flags first
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Trigger Joypad interrupt
    TriggerInterrupt(0x10);  // Set Joypad flag
    
    // Execute a few cycles to allow interrupt handling
    ExecuteInstructions(10);
    
    // Verify interrupt flag was cleared after handling
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x10) != 0) {
        throw std::runtime_error("Joypad interrupt flag not cleared after handling");
    }
    
    LOG("    ✓ Joypad interrupt test passed");
}

void InterruptTest::TestInterruptPriority()
{
    LOG("    Testing interrupt priority...");
    
    // Create a simple program that enables interrupts and waits
    std::vector<byte> program = {
        0xFB,      // EI (enable interrupts)  
        0x00,      // NOP
        0x76       // HALT
    };
    
    WriteProgramToCartridge(program);
    
    // Enable all interrupts
    _bus->Write(0xFFFF, 0x1F);  // IE: Enable all
    
    // Clear any existing interrupt flags first
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Set multiple interrupt flags (VBlank has highest priority)
    _bus->Write(AddressConstants::InterruptFlag, 0x1F);  // All interrupts pending
    
    // Execute a few cycles to allow interrupt handling
    ExecuteInstructions(10);
    
    // VBlank should be handled first (highest priority) - verify VBlank flag was cleared
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x01) != 0) {
        throw std::runtime_error("Interrupt priority test failed: VBlank interrupt (highest priority) not handled first");
    }
    
    LOG("    ✓ Interrupt priority test passed");
}

void InterruptTest::TestInterruptMasterEnable()
{
    LOG("    Testing Interrupt Master Enable...");
    
    // Create a simple program that starts with interrupts disabled
    std::vector<byte> program = {
        0xF3,      // DI (disable interrupts)
        0x00,      // NOP
        0x00,      // NOP
        0xFB,      // EI (enable interrupts)
        0x00,      // NOP
        0x76       // HALT
    };
    
    WriteProgramToCartridge(program);
    
    // Enable VBlank interrupt
    _bus->Write(0xFFFF, 0x01);  // IE: Enable VBlank
    
    // Clear any existing interrupt flags from post-boot state
    _bus->Write(AddressConstants::InterruptFlag, 0x00);
    
    // Trigger VBlank interrupt before enabling IME
    TriggerInterrupt(0x01);  // Set VBlank flag
    
    // Verify flag was set
    byte interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x01) == 0) {
        throw std::runtime_error("Interrupt flag was not set by TriggerInterrupt");
    }
    
    // Execute just 3 instructions (DI, NOP, NOP) - interrupt should NOT be handled while IME is disabled
    ExecuteInstructions(3);
    
    // Check current IME state  
    bool imeState = _cpu->GetIME();
    
    // Interrupt flag should still be set (not processed while IME disabled)
    interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x01) == 0) {
        throw std::runtime_error("Interrupt flag was cleared while IME was disabled. IME state: " + 
                                std::to_string(imeState));
    }
    
    // Execute more instructions to reach EI and allow 1-instruction delay, then process interrupt
    ExecuteInstructions(5);  // EI (instruction 4) + NOP (instruction 5, IME enabled) + some cycles for processing
    
    // Verify interrupt was handled after EI (flag should be cleared)
    interruptFlag = _bus->Read(AddressConstants::InterruptFlag);
    if ((interruptFlag & 0x01) != 0) {
        throw std::runtime_error("Interrupt not handled after enabling IME");
    }
    
    LOG("    ✓ Interrupt Master Enable test passed");
}

void InterruptTest::TestInterruptHandlerAddresses()
{
    LOG("    Testing interrupt handler addresses...");
    
    // Test that all interrupt handler addresses are correct
    if (AddressConstants::VBlankHandlerAddress != 0x40) {
        throw std::runtime_error("VBlank handler address incorrect");
    }
    
    if (AddressConstants::LcdHandlerAddress != 0x48) {
        throw std::runtime_error("LCD handler address incorrect");
    }
    
    if (AddressConstants::TimerHandlerAddress != 0x50) {
        throw std::runtime_error("Timer handler address incorrect");
    }
    
    if (AddressConstants::SerialHandlerAddress != 0x58) {
        throw std::runtime_error("Serial handler address incorrect");
    }
    
    if (AddressConstants::JoypadHandlerAddress != 0x60) {
        throw std::runtime_error("Joypad handler address incorrect");
    }
    
    // Test that we can read from these addresses (our pre-installed handlers)
    if (_bus->Read(AddressConstants::VBlankHandlerAddress) != 0xC9) {
        throw std::runtime_error("VBlank handler not properly installed");
    }
    
    if (_bus->Read(AddressConstants::LcdHandlerAddress) != 0xC9) {
        throw std::runtime_error("LCD handler not properly installed");
    }
    
    LOG("    ✓ Interrupt handler addresses test passed");
}

void InterruptTest::WriteProgramToCartridge(const std::vector<byte>& program)
{
    // Update cartridge data with the program starting at 0x0100 (where PC starts)
    for (size_t i = 0; i < program.size(); ++i) {
        _cartridgeData[0x0100 + i] = program[i];
    }
    
    // Recreate the cartridge and bus with updated data
    _cartridge = std::make_unique<Cartridge>(_cartridgeData);
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), 
                                _wRam.get(), _wRamCgb.get(), _echoRam.get(), 
                                _oam.get(), _ioRegisters.get(), _hRam.get());
    _cpu = std::make_unique<TestCpu>(_bus.get());
    
    // Re-initialize to proper post-boot state
    InitializePostBootHardwareState(_bus.get());
    _cpu->InitializePostBootState();
}

void InterruptTest::TriggerInterrupt(byte interruptBit)
{
    // Set the specific interrupt flag
    byte currentFlags = _bus->Read(AddressConstants::InterruptFlag);
    _bus->Write(AddressConstants::InterruptFlag, currentFlags | interruptBit);
}

void InterruptTest::VerifyInterruptTriggered(word expectedPc, const std::string& interruptName)
{
    word actualPc = _cpu->GetPC();
    if (actualPc != expectedPc) {
        throw std::runtime_error(interruptName + " interrupt verification failed: expected PC=0x" + 
                                std::to_string(expectedPc) + ", got PC=0x" + std::to_string(actualPc));
    }
}

void InterruptTest::ExecuteInstructions(int count)
{
    for (int i = 0; i < count; i++) {
        _cpu->Update();
    }
}