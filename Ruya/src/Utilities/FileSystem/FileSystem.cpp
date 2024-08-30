#include "FileSystem.h"
#include <fstream>
#include "../Log/RLog.h"


std::vector<char> Ruya::ReadBinaryFile(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);

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

bool Ruya::ImportOBJMesh(Mesh& mesh, const std::string& filepath)
{
	return true;
}


