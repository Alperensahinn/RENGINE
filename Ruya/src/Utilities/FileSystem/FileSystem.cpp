#include "FileSystem.h"
#include <fstream>
#include "../Log/RLog.h"


namespace Ruya 
{
    std::vector<char> ReadBinaryFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) 
        {
            RERRLOG("[File System] Failed to open file.")
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}

