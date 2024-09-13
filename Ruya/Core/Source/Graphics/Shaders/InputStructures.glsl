
layout(set = 0, binding = 0) uniform  SceneData
{   
	mat4 view;
	mat4 proj;
	mat4 projView;
} sceneData;

layout(set = 1, binding = 0) uniform sampler2D albedoTexture;
