#version 300 es
precision highp float;

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

uniform bool hdr;
uniform float exposure;
uniform float bloomStrength; // range (0.03, 0.15) works really well.

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, texCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, texCoords).rgb;
    vec3 mixed_color = mix(hdrColor, bloomColor, bloomStrength); // linear interpolation;

    if(hdr)
    {
        // exposure
        vec3 result = vec3(1.0) - exp(-mixed_color * exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma));
        fragColor = vec4(result, 1.0);
    }
    else
    {
        vec3 result = pow(mixed_color, vec3(1.0 / gamma));
        fragColor = vec4(result, 1.0);
    }
}