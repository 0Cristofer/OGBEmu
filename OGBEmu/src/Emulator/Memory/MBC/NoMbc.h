#pragma once

#include <vector>

#include "BaseMbc.h"

class NoMbc final : public BaseMbc
{
public:
    explicit NoMbc(std::vector<byte>* rom);
    ~NoMbc() override;
    
    byte Read(word address) override;
    void Write(word address, byte data) override;

private:
    static word TranslateAddress(word address);

    std::vector<byte>* _rom = nullptr;
    std::vector<byte> _ram;
};
