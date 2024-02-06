#version 300 es
precision highp float;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBiTangent;

out vec2 texCoords;
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 normalMatrix;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec3 fragPos = vec3(worldPos);
    texCoords = aTexCoords;

    mat3 normalMatrix = mat3(normalMatrix);
    vec3 T = normalize(normalMatrix * normalize(aTangent));
    vec3 N = normalize(normalMatrix * normalize(aNormal));
    vec3 B = normalize(normalMatrix * normalize(aBiTangent));

    mat3 TBN = transpose(mat3(T, B, N)); //inverting

    //TBN translates from world space to tangent space
    tangentLightPos = TBN * lightPos;
    tangentViewPos  = TBN * viewPos;
    tangentFragPos  = TBN * fragPos;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}