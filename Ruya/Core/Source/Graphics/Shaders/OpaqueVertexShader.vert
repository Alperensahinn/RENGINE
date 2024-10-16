#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) out vec2 outTexCoord;

void main()
{
     vec2 positions[6] = vec2[]
    (
        vec2(-1.0,  1.0),  
        vec2(-1.0, -1.0),  
        vec2( 1.0,  1.0),
        
        vec2( 1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0, -1.0)
    );

    
    vec2 texCoords[6] = vec2[](
        vec2(0.0, 1.0),  
        vec2(0.0, 0.0),  
        vec2(1.0, 1.0),  

        vec2(1.0, 0.0),  
        vec2(1.0, 1.0),
        vec2(0.0, 0.0)
    );

    vec4 pos = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    gl_Position = pos;
    outTexCoord = texCoords[gl_VertexIndex];
}