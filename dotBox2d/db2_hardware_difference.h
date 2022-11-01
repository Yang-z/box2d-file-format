#pragma once

class hardwareDifference
{
public:
    static auto getDataStructureAlignment(bool packed = false) -> long long;

    static auto isLittleEndian() -> bool;
    static auto reverseEndian(char *source, unsigned long long length) -> void;
};
