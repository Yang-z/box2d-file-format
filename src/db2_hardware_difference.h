#pragma once

#include "db2_settings.h"

class hardwareDifference
{
public:
    static auto GetDataStructureAlignment(const bool packed = false) -> uint8_t;

    static constexpr auto IsLittleEndian() -> const bool { return std::endian::native == std::endian::little; };
    static constexpr auto IsBigEndian() -> const bool { return std::endian::native == std::endian::big; };
    static auto ReverseEndian(char *source, const uint8_t length) -> void;

    static auto IsLittleEndian_Bit() -> const bool;

    static constexpr auto IEEE754() -> const bool;

    static constexpr auto Check() -> const bool;
};
