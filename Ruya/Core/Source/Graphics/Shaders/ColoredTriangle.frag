#version 450

#include "InputStructures.glsl"

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 color =  texture(albedoTexture,inUV).xyz;

	outFragColor = vec4(color, 1.0f);
}
