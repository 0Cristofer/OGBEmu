#include "DmaTest.h"
#include "Core/Logger.h"
#include "Emulator/GbConstants.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>

DmaTest::DmaTest() : BaseTest("DMA Test")
{
}

void DmaTest::Run()
{
    LOG("  Testing DMA functionality...");
    
    TestDmaTransfer();
    TestDmaFromDifferentSources();
    TestDmaActive();
    TestDmaCycles();
    TestDmaReset();
    
    LOG("  DMA test completed successfully!");
}

void DmaTest::Setup()
{
    LOG("  Setting up DMA test...");
    
    // Create minimal boot ROM
    std::vector<byte> bootRomData(256, 0x00);
    
    // Create cartridge data
    _cartridgeData.resize(0x8000, 0x00); // 32KB ROM
    _cartridgeData[0x0147] = 0x00; // ROM only cartridge type
    _cartridgeData[0x0148] = 0x00; // 32KB ROM size (0x00 = 32KB, 0x01 = 64KB)
    _cartridgeData[0x0149] = 0x00; // No RAM
    
    // Create memory components
    _bootRom = std::make_unique<BootRom>(bootRomData);
    _cartridge = std::make_unique<Cartridge>(_cartridgeData);
    _vRam = std::make_unique<VRam>();
    _wRam = std::make_unique<WRam>();
    _wRamCgb = std::make_unique<WRamCgb>();
    _echoRam = std::make_unique<EchoRam>();
    _oam = std::make_unique<Oam>();
    _ioRegisters = std::make_unique<IoRegisters>();
    _hRam = std::make_unique<HRam>();
    
    // Create bus
    _bus = std::make_unique<Bus>(_bootRom.get(), _cartridge.get(), _vRam.get(), _wRam.get(),
                                 _wRamCgb.get(), _echoRam.get(), _oam.get(), _ioRegisters.get(),
                                 _hRam.get());
    
    // Create DMA
    _dma = std::make_unique<Dma>(_bus.get());
    
    // Set DMA pointer in Bus
    _bus->SetDma(_dma.get());
    
    // Disable boot ROM
    _bus->Write(AddressConstants::BootRomBank, 0x01);
    
    LOG("  DMA test setup complete!");
}

void DmaTest::TestDmaTransfer()
{
    LOG("    Testing basic DMA transfer...");
    
    // Fill WRAM with test data
    FillSourceData(0xC000, 160);
    
    // Clear OAM
    for (int i = 0; i < 160; i++) {
        WriteMemory(AddressConstants::StartOamAddress + i, 0x00);
    }
    
    // Start DMA transfer from WRAM (0xC000)
    WriteMemory(AddressConstants::DmaStart, 0xC0);
    
    // DMA should be active
    if (!_dma->IsActive()) {
        throw std::runtime_error("DMA should be active after start");
    }
    
    // Complete DMA transfer
    int totalCycles = 0;
    while (_dma->IsActive()) {
        totalCycles += SimulateDmaUpdate();
        if (totalCycles > 1000) { // Safety check
            throw std::runtime_error("DMA transfer took too long");
        }
    }
    
    // Verify transfer
    VerifyOamTransfer(0xC000);
    
    LOG("    Basic DMA transfer test passed!");
}

void DmaTest::TestDmaFromDifferentSources()
{
    LOG("    Testing DMA from different source addresses...");
    
    // Test DMA from ROM (0x0000-0x7FFF)
    // Note: ROM is read-only, so we test DMA with existing ROM data (all 0x00)
    WriteMemory(AddressConstants::DmaStart, 0x00);
    int totalCycles = 0;
    while (_dma->IsActive()) {
        totalCycles += SimulateDmaUpdate();
        if (totalCycles > 1000) { // Safety check
            throw std::runtime_error("DMA ROM transfer took too long");
        }
    }
    VerifyOamTransferFromRom(0x0000);
    
    // Test DMA from VRAM (0x8000-0x9FFF)
    FillSourceData(0x8000, 160);
    WriteMemory(AddressConstants::DmaStart, 0x80);
    totalCycles = 0;
    while (_dma->IsActive()) {
        totalCycles += SimulateDmaUpdate();
        if (totalCycles > 1000) { // Safety check
            throw std::runtime_error("DMA VRAM transfer took too long");
        }
    }
    VerifyOamTransfer(0x8000);
    
    // Test DMA from WRAM (0xC000-0xDFFF)
    FillSourceData(0xC000, 160);
    WriteMemory(AddressConstants::DmaStart, 0xC0);
    totalCycles = 0;
    while (_dma->IsActive()) {
        totalCycles += SimulateDmaUpdate();
        if (totalCycles > 1000) { // Safety check
            throw std::runtime_error("DMA WRAM transfer took too long");
        }
    }
    VerifyOamTransfer(0xC000);
    
    // Test DMA from Echo RAM (0xE000-0xFDFF)
    FillSourceData(0xE000, 160);
    WriteMemory(AddressConstants::DmaStart, 0xE0);
    totalCycles = 0;
    while (_dma->IsActive()) {
        totalCycles += SimulateDmaUpdate();
        if (totalCycles > 1000) { // Safety check
            throw std::runtime_error("DMA Echo RAM transfer took too long");
        }
    }
    VerifyOamTransfer(0xE000);
    
    LOG("    DMA from different sources test passed!");
}

void DmaTest::TestDmaActive()
{
    LOG("    Testing DMA active state...");
    
    // Initially not active
    if (_dma->IsActive()) {
        throw std::runtime_error("DMA should not be active initially");
    }
    
    // Fill source data
    FillSourceData(0xC000, 160);
    
    // Start DMA
    WriteMemory(AddressConstants::DmaStart, 0xC0);
    
    // Should be active now
    if (!_dma->IsActive()) {
        throw std::runtime_error("DMA should be active after start");
    }
    
    // Transfer some bytes but not all
    for (int i = 0; i < 80; i++) { // Half transfer
        int cycles = SimulateDmaUpdate();
        if (cycles == 0) break; // DMA finished early
    }
    
    // Should still be active if not finished
    if (!_dma->IsActive()) {
        // This is okay if transfer completed in fewer updates
        LOG("      DMA completed faster than expected");
    }
    
    // Complete transfer
    int safetyCycles = 0;
    while (_dma->IsActive()) {
        SimulateDmaUpdate();
        safetyCycles++;
        if (safetyCycles > 200) { // Safety check
            throw std::runtime_error("DMA active state test took too long");
        }
    }
    
    // Should not be active anymore
    if (_dma->IsActive()) {
        throw std::runtime_error("DMA should not be active after completion");
    }
    
    LOG("    DMA active state test passed!");
}

void DmaTest::TestDmaCycles()
{
    LOG("    Testing DMA cycle consumption...");
    
    // Fill source data
    FillSourceData(0xC000, 160);
    
    // Start DMA
    WriteMemory(AddressConstants::DmaStart, 0xC0);
    
    // Count cycles consumed
    int totalCycles = 0;
    int safetyUpdates = 0;
    while (_dma->IsActive()) {
        int cycles = SimulateDmaUpdate();
        if (cycles == 0) {
            throw std::runtime_error("DMA update should consume cycles");
        }
        totalCycles += cycles;
        safetyUpdates++;
        if (safetyUpdates > 200) { // Safety check
            throw std::runtime_error("DMA timing test took too long");
        }
    }
    
    // DMA should consume 160 * 4 = 640 cycles (4 cycles per byte)
    if (totalCycles != 640) {
        std::ostringstream oss;
        oss << "DMA should consume 640 cycles, got " << totalCycles;
        throw std::runtime_error(oss.str());
    }
    
    LOG("    DMA cycle consumption test passed!");
}

void DmaTest::TestDmaReset()
{
    LOG("    Testing DMA reset...");
    
    // Fill source data and start DMA
    FillSourceData(0xC000, 160);
    WriteMemory(AddressConstants::DmaStart, 0xC0);
    
    // Verify DMA is active
    if (!_dma->IsActive()) {
        throw std::runtime_error("DMA should be active before reset");
    }
    
    // Reset DMA
    _dma->Reset();
    
    // DMA should not be active after reset
    if (_dma->IsActive()) {
        throw std::runtime_error("DMA should not be active after reset");
    }
    
    // Update should not consume cycles
    int cycles = SimulateDmaUpdate();
    if (cycles != 0) {
        throw std::runtime_error("DMA update should not consume cycles after reset");
    }
    
    LOG("    DMA reset test passed!");
}

void DmaTest::AssertMemory(word address, byte expected, const std::string& message)
{
    byte actual = _bus->Read(address);
    if (actual != expected)
    {
        std::ostringstream oss;
        oss << message << " - Expected: 0x" << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(expected) << ", Got: 0x" << static_cast<int>(actual);
        throw std::runtime_error(oss.str());
    }
}

void DmaTest::WriteMemory(word address, byte value)
{
    _bus->Write(address, value);
}

void DmaTest::FillSourceData(word startAddress, int size)
{
    for (int i = 0; i < size; i++) {
        WriteMemory(startAddress + i, static_cast<byte>((i + 1) & 0xFF));
    }
}

void DmaTest::VerifyOamTransfer(word sourceAddress)
{
    for (int i = 0; i < 160; i++) {
        byte expected = _bus->Read(sourceAddress + i);
        AssertMemory(AddressConstants::StartOamAddress + i, expected, 
                    "OAM transfer verification at offset " + std::to_string(i));
    }
}

void DmaTest::VerifyOamTransferFromRom(word sourceAddress)
{
    // ROM data is all 0x00, so we verify OAM contains 0x00 after DMA from ROM
    for (int i = 0; i < 160; i++) {
        AssertMemory(AddressConstants::StartOamAddress + i, 0x00, 
                    "OAM ROM transfer verification at offset " + std::to_string(i));
    }
}

int DmaTest::SimulateDmaUpdate()
{
    return _dma->Update();
}