#version 300 es
precision highp float;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;

out vec2 texCoords;
out vec3 fragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 normalMatrix;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 viewpos = view * worldPos;
    fragPos = vec3(viewpos);
    texCoords = aTexCoords;

    mat3 normalMatrix = mat3(normalMatrix);

    vec3 T = normalize(normalMatrix * normalize(aTangent));
    vec3 N = normalize(normalMatrix * normalize(aNormal));
    T = normalize(T - dot(T, N) * N); //reorthogonalize the tangent
    vec3 B = normalize(cross(N, T));

    TBN = mat3(T, B, N);

    gl_Position = projection * view * worldPos;

}