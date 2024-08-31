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

std::shared_ptr<Ruya::Mesh> Ruya::ImportFBXMesh(const std::string& filepath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

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



