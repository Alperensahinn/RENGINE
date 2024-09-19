#include "FileSystem.h"
#include <fstream>
#include "../Log/RLog.h"
#include <stb_image.h>
#include <Engine.h>


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

std::shared_ptr<Ruya::Mesh> Ruya::ImportFBXMesh(const std::string& filepath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		printf("%s", importer.GetErrorString());
		return nullptr;
	}

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
	auto processedData = ProcessNode(scene->mRootNode, scene);
	mesh->vertices = processedData.vertices;
	mesh->indices = processedData.indices;

	return mesh;
}

Ruya::Mesh Ruya::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		return ProcessMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		return ProcessNode(node->mChildren[i], scene);
	}
}

Ruya::Mesh Ruya::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		math::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.tangent = vector;

		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		vertex.biTangent = vector;

		if (mesh->mTextureCoords[0])
		{
			math::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.uv_x = vec.x;
			vertex.uv_y = vec.y;
		}

		else
		{
			vertex.uv_x = 0.0f;
			vertex.uv_y = 0.0f;
		}

		vertices.push_back(vertex);
	}
	
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}



	return Mesh(vertices, indices);
}

Ruya::Texture Ruya::LoadTexture(std::string path, VkFormat format)
{
	Ruya::Texture texture;

	int width, height, channels;
	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		RERRLOG("[File System] Failed to load texture image: " << path);
		return texture;
	}

	VkExtent3D imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

	texture.image = Ruya::rvkCreateImage(Ruya::Engine::GetInstance().GetRenderer().pRVulkan, pixels, imageExtent, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	texture.sampler = Ruya::Engine::GetInstance().GetRenderer().GetRendererBackend()->defaultLinearSampler;

	stbi_image_free(pixels);

	return texture;
}



