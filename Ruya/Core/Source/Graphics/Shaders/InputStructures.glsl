
layout(std140, set = 0, binding = 0) uniform  SceneData
{   
	mat4 view;
	mat4 proj;
	mat4 projView;
	vec3 viewPos;
} sceneData;

layout(set = 1, binding = 0) uniform sampler2D albedoTexture;
layout(set = 1, binding = 1) uniform sampler2D normalTexture;