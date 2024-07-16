#pragma once

#include "db2_settings.h"

class HardwareDifference
{
public:
    static auto GetDataStructureAlignment(const bool packed = false) -> const uint8_t;

    static auto IsLittleEndian() -> const bool;
    static auto IsBigEndian() -> const bool;
    static auto ReverseEndian(char *source, const uint8_t length) -> void;

    static auto IsLittleEndian_Bit() -> const bool;

    static auto IsIEEE754() -> const bool;

    static auto Check() -> const bool;
};
