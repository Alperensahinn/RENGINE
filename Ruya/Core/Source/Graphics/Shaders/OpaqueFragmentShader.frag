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
    vec3 normal =  texture(gNormal,inTexCoord).xyz;
    vec3 fragPos =  texture(gPos,inTexCoord).xyz;
    
    //light
    vec3 lightPos = vec3(10.0f, -5.0f, 20.0f);
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

    //ambient
	float ambientStrength = 0.05;
    vec3 ambient = ambientStrength * lightColor;

    //directional
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //specular
    float specularStrength = 1.0;
    vec3 viewDir = normalize(sceneData.viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  

    vec3 result = (ambient + diffuse + specular) * color;

    outFragColor = vec4(result, 1.0);
}