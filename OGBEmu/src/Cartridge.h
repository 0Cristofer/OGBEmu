#pragma once

#include <string>
#include <vector>

class Cartridge
{
public:
    explicit Cartridge(const std::string& romPath);

    [[nodiscard]] bool IsValid() const { return !_bytes.empty(); }

private:
    static std::vector<unsigned char> ReadRom(const std::string& romPath);

    std::vector<unsigned char> _bytes;
};
