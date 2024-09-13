#version 450
#extension GL_KHR_vulkan_glsl : enable

#include "InputStructures.glsl"

layout (location = 0) out vec2 outTexCoord;

void main()
{
    vec2 positions[4] = vec2[]
    (
        vec2(-1.0,  1.0),  
        vec2(-1.0, -1.0),  
        vec2( 1.0,  1.0),  
        vec2( 1.0, -1.0)   
    );

    
    vec2 texCoords[4] = vec2[](
        vec2(0.0, 1.0),  
        vec2(0.0, 0.0),  
        vec2(1.0, 1.0),  
        vec2(1.0, 0.0)   
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    outTexCoord = texCoords[gl_VertexIndex];
}