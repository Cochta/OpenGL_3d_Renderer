#version 300 es
precision highp float;

layout (location = 0) out float fragColor;

in vec2 texCoords;

uniform sampler2D g_position_metallic;
uniform sampler2D g_normal_roughness;
uniform sampler2D texNoise;

const int kSampleCount = 64;
uniform float radius; // 0.5
uniform float bias; // 0.025

uniform vec3 samples[kSampleCount];

// Tile noise texture over screen based on screen dimensions divided by noise size
uniform vec2 noiseScale;

uniform mat4 projection;

void main()
{
    // get input for SSAO algorithm
    vec3 fragPos = texture(g_position_metallic, texCoords).rgb;
    vec3 normal = normalize(texture(g_normal_roughness, texCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, texCoords * noiseScale).rgb);

    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kSampleCount; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = texture(g_position_metallic, offset.xy).z; // get depth value of kernel sample

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / float(kSampleCount));

    fragColor = occlusion;
}