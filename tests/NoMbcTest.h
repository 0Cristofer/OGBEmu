#pragma once

#include "BaseTest.h"
#include "Emulator/Memory/MBC/NoMbc.h"
#include <vector>

class NoMbcTest : public BaseTest
{
public:
    NoMbcTest();
    
    void Setup() override;
    void Run() override;

private:
    void TestNoMbcRomRead();
    void TestNoMbcRomReadInvalidAddress();
    void TestNoMbcRamWrite();
    void TestNoMbcRamWriteInvalidAddress();
    void TestNoMbcRamSizeDetection();
    void TestNoMbcAddressTranslation();
    void TestNoMbcRamConfiguration();
    void TestNoMbcEdgeCases();
    
    void CreateValidRom();
    void CreateValidRomWithRam();
    void CreateValidRomWithNoRam();
    void CreateInvalidRom();
    
    std::vector<byte> _romData;
};