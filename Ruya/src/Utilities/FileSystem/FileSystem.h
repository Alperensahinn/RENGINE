#pragma once
#include <vector>
#include <string>
#include "../../Scene/Model.h"

namespace Ruya
{
	std::vector<char> ReadBinaryFile(const std::string& filepath);

	bool ImportOBJMesh(Mesh& mesh, const std::string& filepath);
}
