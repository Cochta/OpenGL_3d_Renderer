#version 300 es
precision highp float;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoords;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform sampler2D g_position_metallic;
uniform sampler2D g_normal_roughness;
uniform sampler2D g_albedo_ao;

// ssao
uniform sampler2D ssao_tex;

//shadow
uniform samplerCube depth_tex;

uniform mat4 lightSpaceMatrix;

uniform float lightFarPlane;

// lights
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 camPos;
uniform mat4 inverse_view;

const float PI = 3.14159265359;

//shadow
float PointLightShadowCalculation(vec3 fragWorldPos, vec3 worldNormal) {
    vec3 fragToLight = fragWorldPos - lightPos;
    float currentDepth = length(fragToLight);
    float closestDepth = texture(depth_tex, fragToLight).r;
    closestDepth *= lightFarPlane;
    // get depth of current fragment from light's perspective

    float shadow  = 0.0;
    float bias = 0.03;
    const float samples = 4.0;
    const float offset  = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            {
                float closestDepth = texture(depth_tex, fragToLight + vec3(x, y, z)).r;
                closestDepth *= lightFarPlane;   // undo mapping [0;1]

                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
            }
        }
    }
    shadow /= (samples * samples * samples);
    return shadow;
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   
// ----------------------------------------------------------------------------
void main()
{		
    // material properties
    vec3 albedo = texture(g_albedo_ao, texCoords).rgb;
    float metallic = texture(g_position_metallic, texCoords).a;
    float roughness = texture(g_normal_roughness, texCoords).a;
    float ao = texture(g_albedo_ao, texCoords).a;

    float ssao = texture(ssao_tex, texCoords).r;
    ao *= ssao;

    // input lighting data
    vec3 N = texture(g_normal_roughness, texCoords).rgb;
    N = mat3(inverse_view) * N;
    vec3 ViewPos = texture(g_position_metallic, texCoords).rgb;
    vec4 WorldPosv4 = inverse_view * vec4(ViewPos,1.0);
    vec3 WorldPos = vec3(WorldPosv4);
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

        // calculate per-light radiance
        vec3 L = normalize(lightPos - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos - WorldPos);
        float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * (distance * distance));  
        vec3 radiance = lightColor * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

float shadow = PointLightShadowCalculation(WorldPos, N);

        // add to outgoing radiance Lo
        Lo += (1.0-shadow)*(kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
     
    
    // ambient lighting (we now use IBL as the ambient term)
     F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
     kS = F;
     kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
     specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

        float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    FragColor = vec4(color , 1.0);
    
}