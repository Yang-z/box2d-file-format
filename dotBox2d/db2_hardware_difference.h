#pragma once

#include "db2_settings.h"

class hardwareDifference
{
public:
    static auto GetDataStructureAlignment(bool packed = false) -> uint8_t;

    static auto IsLittleEndian() -> bool;
    static auto IsBigEndian() -> bool;
    static auto ReverseEndian(char *source, uint8_t length) -> void;

    static auto IsLittleEndian_Bit() -> bool;

    static auto IEEE754() -> bool;

    static auto Check() -> bool;
};
