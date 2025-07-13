#pragma once

#include "BaseTest.h"
#include "TestCpu.h"
#include "Emulator/Dma.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/WRam.h"
#include "Emulator/Memory/WRamCgb.h"
#include "Emulator/Memory/EchoRam.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/HRam.h"
#include "Emulator/Memory/AddressConstants.h"
#include <memory>

class DmaTest : public BaseTest
{
public:
    DmaTest();
    void Setup() override;
    void Run() override;

private:
    void TestDmaTransfer();
    void TestDmaFromDifferentSources();
    void TestDmaActive();
    void TestDmaCycles();
    void TestDmaReset();
    
    // Helper methods
    void AssertMemory(word address, byte expected, const std::string& message);
    void WriteMemory(word address, byte value);
    void FillSourceData(word startAddress, int size);
    void VerifyOamTransfer(word sourceAddress);
    void VerifyOamTransferFromRom(word sourceAddress);
    int SimulateDmaUpdate();
    
    // Memory components
    std::unique_ptr<BootRom> _bootRom;
    std::unique_ptr<Cartridge> _cartridge;
    std::unique_ptr<VRam> _vRam;
    std::unique_ptr<WRam> _wRam;
    std::unique_ptr<WRamCgb> _wRamCgb;
    std::unique_ptr<EchoRam> _echoRam;
    std::unique_ptr<Oam> _oam;
    std::unique_ptr<IoRegisters> _ioRegisters;
    std::unique_ptr<HRam> _hRam;
    std::unique_ptr<Bus> _bus;
    std::unique_ptr<Dma> _dma;
    
    std::vector<byte> _cartridgeData;
};