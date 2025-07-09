#pragma once

#include "BaseTest.h"
#include "Emulator/Memory/BootRom.h"
#include "Emulator/Memory/Cartridge.h"
#include "Emulator/Memory/Bus.h"
#include "Emulator/Memory/VRam.h"
#include "Emulator/Memory/WRam.h"
#include "Emulator/Memory/WRamCgb.h"
#include "Emulator/Memory/EchoRam.h"
#include "Emulator/Memory/Oam.h"
#include "Emulator/Memory/IoRegisters.h"
#include "Emulator/Memory/HRam.h"
#include "Emulator/Cpu.h"
#include "MockScreen.h"
#include "Core/Utils.h"
#include <memory>

class DeviceTest : public BaseTest
{
public:
    DeviceTest();
    
    void Setup() override;
    void Run() override;

private:
    void TestDeviceComponentInitialization();
    void TestDeviceValidation();
    void TestFrameTimingCalculation();
    void TestBootRomValidation();
    void TestCartridgeValidation();
    void TestComponentIntegration();
    void TestCpuAndMemoryIntegration();
    void TestSystemComponentsValidation();
    
    void CreateValidBootRom();
    void CreateValidCartridge();
    void CreateInvalidBootRom();
    void CreateInvalidCartridge();
    bool SimulateDeviceValidation(const std::vector<byte>& bootRom, const std::vector<byte>& cartridge);
    
    std::vector<byte> _bootRomData;
    std::vector<byte> _cartridgeData;
    std::unique_ptr<MockScreen> _mockScreen;
};