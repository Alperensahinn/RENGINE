#version 450

#include "InputStructures.glsl"

layout (location = 0) in vec4 infragPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in mat3 inTBN;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outFragPos;
layout (location = 2) out vec4 outFragNormal;

void main() 
{
    vec3 color = texture(albedoTexture, inUV).xyz;
    outFragColor = vec4(color, 1.0f);

    outFragPos = infragPos;

    vec3 norm = texture(normalTexture, inUV).xyz;
    norm = norm * 2.0 - 1.0;
    outFragNormal = vec4(normalize(inTBN * norm), 1.0f);
}
