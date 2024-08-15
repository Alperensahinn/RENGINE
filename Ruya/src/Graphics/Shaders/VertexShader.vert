#version 450
#extension GL_KHR_vulkan_glsl : enable

vec2[3] positions = vec2[]
(
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
    fragColor = colors[gl_VertexIndex];
}