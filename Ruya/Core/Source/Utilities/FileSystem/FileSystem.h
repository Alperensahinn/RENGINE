#pragma once
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../../Graphics/Mesh.h"
#include <memory>
#include <Graphics/Renderer/Texture.h>

namespace Ruya
{
	std::vector<char> ReadBinaryFile(const std::string& filepath);

	std::shared_ptr<Ruya::Mesh> ImportFBXMesh(const std::string& filepath);

	Mesh ProcessNode(aiNode* node, const aiScene* scene);

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	Texture LoadTexture(std::string path);
}
