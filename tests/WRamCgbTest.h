#pragma once

#include "BaseTest.h"
#include "Emulator/Memory/WRamCgb.h"

class WRamCgbTest : public BaseTest
{
public:
    WRamCgbTest();
    
    void Setup() override;
    void Run() override;

private:
    void TestWRamCgbBasicReadWrite();
    void TestWRamCgbAddressTranslation();
    void TestWRamCgbBoundaryConditions();
    void TestWRamCgbInvalidAddresses();
    void TestWRamCgbDataIntegrity();
    void TestWRamCgbSequentialAccess();
    void TestWRamCgbRandomAccess();
    void TestWRamCgbConstants();
    
    WRamCgb _wRamCgb;
};