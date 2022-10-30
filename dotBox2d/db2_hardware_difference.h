#pragma once

class hardwareDifference
{
public:
    static auto getDataStructureAlignment(bool packed = false) -> long long;

    static auto isBigEndian() -> bool;
    static auto reverseEndian(char *source, int length) -> void;
};
