#version 300 es
precision highp float;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoords;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

uniform float shininess;

void main()
{
    // Normal
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0); //[0,1] -> [-1,1]

    // Texture color.
    vec3 tex_diffuse = texture(diffuseMap, texCoords).rgb;
    vec3 tex_specular = texture(specularMap, texCoords).rgb;

    // ambient
    vec3 ambient = tex_diffuse * 0.1;

    // diffuse
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos); // direction to the light.

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = tex_diffuse * diff;

    // specular
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    vec3 halfWayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(normal, halfWayDir), 0.0), shininess);
    vec3 specular = tex_specular * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}