#version 450

#include "InputStructures.glsl"

layout (location = 0) in vec4 infragPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in mat3 inTBN;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outFragPos;
layout (location = 2) out vec4 outFragNormal;
layout (location = 3) out vec4 outFragRoughness;
layout (location = 4) out vec4 outFragMetalic;

void main() 
{
    //albedo
    vec3 color = texture(albedoTexture, inUV).xyz;
    outFragColor = vec4(color, 1.0f);

    //world pos
    outFragPos = infragPos;

    //roughness metalic
    vec3 roughnessMetalic = texture(roughnessMetalicTexture, inUV).xyz;
    outFragRoughness = vec4(roughnessMetalic.g, roughnessMetalic.g, roughnessMetalic.g, 1.0f);
    outFragMetalic = vec4(roughnessMetalic.b, roughnessMetalic.b, roughnessMetalic.b, 1.0f);

    //normal
    vec3 norm = texture(normalTexture, inUV).xyz;
    norm = norm * 2.0 - 1.0;
    outFragNormal = vec4(normalize(inTBN * norm), 1.0f);
}
