#version 450

#include "InputStructures.glsl"

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 0) uniform sampler2D gAlbedo;
layout(set = 1, binding = 1) uniform sampler2D gPos;
layout(set = 1, binding = 2) uniform sampler2D gNormal;
layout(set = 1, binding = 3) uniform sampler2D gRoughness;
layout(set = 1, binding = 4) uniform sampler2D gMetallic;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


void main() 
{
    vec3 albedo = texture(gAlbedo, inTexCoord).rgb;
    vec3 normal = texture(gNormal,inTexCoord).xyz;
    vec3 fragPos = texture(gPos,inTexCoord).xyz;
    vec3 roughness =  texture(gRoughness,inTexCoord).xyz;
    vec3 metallic =  texture(gMetallic,inTexCoord).xyz;

    vec3 N = normalize(normal);
    vec3 V = normalize(sceneData.viewPos - fragPos); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    vec3 lightPos = vec3(3.0f, 3.0f, 3.0f);
    vec3 lightColor = vec3(20, 20, 20);

    //pbr calculations
    vec3 Lo = vec3(0.0);

    vec3 L = normalize(lightPos - fragPos);
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor;      
        
    float NDF = DistributionGGX(N, H, roughness.x);        
    float G = GeometrySmith(N, V, L, roughness.x);      
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;  
        
    float NdotL = max(dot(N, L), 0.0);                
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; 

    vec3 ambient = vec3(0.1) * albedo;
    vec3 result = ambient + Lo;  

    result = result / (result + vec3(1.0));

    outFragColor = vec4(result, 1.0);
}