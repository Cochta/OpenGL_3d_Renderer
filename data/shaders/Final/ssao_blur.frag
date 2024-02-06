#version 300 es
precision highp float;

layout (location = 0) out float fragColor;

in vec2 texCoords;

uniform sampler2D ssao_tex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssao_tex, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssao_tex, texCoords + offset).r;
        }
    }

    fragColor = result / (4.0 * 4.0);
}