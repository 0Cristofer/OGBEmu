#pragma once

#include <vector>

#include "BaseMbc.h"

class Mbc1 : public BaseMbc
{
public:
    explicit Mbc1(std::vector<byte>* rom);
    
    byte Read(word address) override;
    void Write(word address, byte data) override;

private:
    std::vector<byte>* _rom;
    std::vector<byte> _ram;

    struct
    {
        byte ramBank:2;
        byte romBank:5;
    } _bankSelect;

    byte _bankingMode;
    byte _ramEnable;
};
