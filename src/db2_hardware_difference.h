#pragma once

#include "db2_settings.h"

class HardwareDifference
{
public:
    static auto GetDataStructureAlignment(const bool packed = false) -> const uint8_t;

    static constexpr auto IsLittleEndian() -> const bool { return std::endian::native == std::endian::little; };
    static constexpr auto IsBigEndian() -> const bool { return std::endian::native == std::endian::big; };
    static auto ReverseEndian(char *source, const uint8_t length) -> void;

    static auto IsLittleEndian_Bit() -> const bool;

    static constexpr auto IsIEEE754() -> const bool;

    static constexpr auto Check() -> const bool;
};
