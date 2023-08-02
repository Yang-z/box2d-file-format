#pragma once

#include "db2_settings.h"

class hardwareDifference
{
public:
    static auto getDataStructureAlignment(bool packed = false) -> uint8_t;

    static auto isLittleEndian() -> bool;
    static auto isBigEndian() -> bool;
    static auto reverseEndian(char *source, uint8_t length) -> void;

    static auto IEEE754() -> bool;

    static auto check() -> bool;
};
