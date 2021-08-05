#ifndef DB2_HARDWARE_DIFFERENCE_H
#define DB2_HARDWARE_DIFFERENCE_H

class hardwareDifference
{
public:
    static auto isBigEndian() -> bool;
    static auto convertLittleToBigEndian(int *i) -> void;
    static auto convertBigToLittleEndian(int *i) -> void;

    static auto getMemoryAlignment() -> long long;
};

#endif