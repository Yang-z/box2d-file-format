#include "db2_data_structure.h"
#include "db2_hardware_difference.h"

#include <fstream>

auto dotBox2d::load(char *filePath) -> void
{
    std::ifstream fs{filePath, std::ios::binary};
    if (!fs)
    {
        //error: open file faild
        return;
    }

    fs.read((char *)&(this->raw), sizeof(this->raw));
    


}
