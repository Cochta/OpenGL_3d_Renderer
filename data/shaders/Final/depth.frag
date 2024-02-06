#version 300 es
precision highp float;

in vec3 fragPos;

uniform vec3 lightPos;

uniform float lightFarPlane;

void main()
{
    vec3 delta = fragPos -lightPos;
    gl_FragDepth = length(delta) / lightFarPlane;
}