#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_buffer_reference : require

#include "InputStructures.glsl"

layout (location = 0) out vec4 outFragPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;

struct Vertex 
{
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

layout( push_constant ) uniform constants
{	
	mat4 modelMatrix;
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	vec4 position = vec4(v.position, 1.0f);

	outFragPos = PushConstants.modelMatrix * position;
	outNormal = mat3(transpose(inverse(PushConstants.modelMatrix))) * v.normal;
	outUV = vec2(v.uv_x, v.uv_y);

	gl_Position = sceneData.projView * PushConstants.modelMatrix * position;
}