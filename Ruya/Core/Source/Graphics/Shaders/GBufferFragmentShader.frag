#version 450

#include "InputStructures.glsl"

layout (location = 0) in vec4 infragPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outFragPos;
layout (location = 2) out vec4 outFragNormal;

void main() 
{
	vec3 color =  texture(albedoTexture,inUV).xyz;

	outFragColor = vec4(color, 1.0f);

	outFragPos = infragPos;

	//outFragNormal = vec4(inNormal, 1.0f);
	outFragNormal = vec4(texture(normalTexture,inUV).xyz, 1.0f);
}