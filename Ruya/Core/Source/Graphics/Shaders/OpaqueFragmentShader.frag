#version 450

#include "InputStructures.glsl"

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 0) uniform sampler2D gAlbedo;
layout(set = 1, binding = 1) uniform sampler2D gPos;
layout(set = 1, binding = 2) uniform sampler2D gNormal;

void main() 
{
	vec3 color =  texture(gAlbedo,inTexCoord).xyz;

	outFragColor = vec4(color, 1.0f);
}