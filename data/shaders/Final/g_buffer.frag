#version 300 es
precision highp float;

layout(location = 0) out vec4 g_position_metallic;
layout(location = 1) out vec4 g_normal_roughness;
layout(location = 2) out vec4 g_albedo_ao;

in vec2 texCoords;
in vec3 fragPos;
in mat3 TBN;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

void main() {
    g_position_metallic.rgb = fragPos;
    g_position_metallic.a = texture(metallicMap, texCoords).r;

    vec3 normalTan = texture(normalMap, texCoords).rgb;
    normalTan = normalTan * 2.0 - 1.0;

    g_normal_roughness.rgb = normalize(TBN * normalTan);
    g_normal_roughness.a = texture(roughnessMap, texCoords).r;

    g_albedo_ao.rgb = texture(albedoMap, texCoords).rgb;
    g_albedo_ao.a = texture(aoMap, texCoords).r;

}