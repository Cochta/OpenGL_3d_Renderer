#version 300 es
precision highp float;

layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

out vec3 fragPos;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    fragPos = vec3(model * vec4(aPos, 1.0));
}  
