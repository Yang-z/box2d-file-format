#pragma once

#include "db2_settings.h"

class hardwareDifference
{
public:
    static bool IsCompatible;

public:
    static auto GetDataStructureAlignment(const bool packed = false) -> uint8_t;

    static auto IsLittleEndian() -> bool;
    static auto IsBigEndian() -> bool;
    static auto ReverseEndian(char *source, const uint8_t length) -> void;

    static auto IsLittleEndian_Bit() -> bool;

    static auto IEEE754() -> bool;

    static auto Check() -> bool;
};
