
#define MaxLights 16

#define PI 3.141592
#define Epsilon 0.00001
#define Dielectric 0.04

struct Light
{
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    float3 Position;    // point light only
    float SpotPower;    // spot light only
    float3 Radiance;    // light radiance
};

struct Material
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Shininess;
    float Metalness;
};

//  Material                F0   (Linear)       F0    (sRGB)
//  Water	                (0.02, 0.02, 0.02)	(0.15, 0.15, 0.15)  	
//  Plastic / Glass (Low)	(0.03, 0.03, 0.03)	(0.21, 0.21, 0.21)	
//  Plastic High            (0.05, 0.05, 0.05)	(0.24, 0.24, 0.24)	
//  Glass (high) / Ruby	    (0.08, 0.08, 0.08)	(0.31, 0.31, 0.31)	
//  Diamond	                (0.17, 0.17, 0.17)	(0.45, 0.45, 0.45)	
//  Iron	                (0.56, 0.57, 0.58)	(0.77, 0.78, 0.78)	
//  Copper	                (0.95, 0.64, 0.54)	(0.98, 0.82, 0.76)	
//  Gold	                (1.00, 0.71, 0.29)	(1.00, 0.86, 0.57)	
//  Aluminium	            (0.91, 0.92, 0.92)	(0.96, 0.96, 0.97)	
//  Silver	                (0.95, 0.93, 0.88)	(0.98, 0.97, 0.95)

uint GetTexture2DMipLevels(Texture2D tex)
{
    uint width, height, levels;
    tex.GetDimensions(0, width, height, levels);
    return levels;
}

uint GetTextureCubeMipLevels(TextureCube tex)
{
    uint width, height, levels;
    tex.GetDimensions(0, width, height, levels);
    return levels;
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd-d) / (falloffEnd - falloffStart));
}

// Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor*roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

//  @brief BRDF diffuse function
float3 BRDF(float4 albedo, float3 Li, float3 R0, float3 normal, float metalness)
{
   
    float3 F = SchlickFresnel(R0, normal, Li);

    //float kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness);
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metalness).r;
    
    float3 diffuseBRDF = kd * albedo.rgb;
    
    return diffuseBRDF;
}

float3 BRDFSpecular(
        float4 albedo, float3 Li, float3 R0, 
        float3 pos, float3 toEye, float3 normal, 
        float cosLi, 
        float cosLo, 
        float cosLh, 
        float shininess
)
{
    

  
    float3 F = SchlickFresnel(R0, normal, Li);
    
    float r = shininess + 1.0f;
    float k = (r * r) / 8.0f;
    float G = GeometrySmith(normal, pos, Li, k);  
    float D = GeometrySchlickGGX(cosLh, shininess);
    
    float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0f * cosLi * cosLo);
    
    return specularBRDF;
}



float3 IBLAmbient(
                    Material mat, 
                    float3 irradiance,
                    float3 specularIrradiance,
                    float2 specularBRDF,
                    float3 normal, float3 Li
)
{
    
    float3 F = SchlickFresnel(mat.FresnelR0, normal, Li);
    
    float3 kd = lerp(1.0 - F, 0.0, mat.Shininess);
    
    float3 diffuseIBL = kd * mat.DiffuseAlbedo.rgb * irradiance;
    
    float3 specularIBL = (mat.FresnelR0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
    
    return diffuseIBL + specularIBL;
}

float3 PBR(
    Light gLights[MaxLights], Material mat, 
    float3 pos, float3 normal, float3 toEye, float cosLo,
    float3 shadowFactor,
    float3 irradiance,
    float3 specularIrradiance,
    float2 specularBRDF
)
{
    
    float sum = 0.0f;
    float3 P = pos;
    float3 Wo = toEye;
    float3 N = normal;
    float dW = 1.0f / MaxLights;
    
    float metalness = mat.Metalness;
    float3 F0 = lerp(Dielectric, mat.DiffuseAlbedo.rgb, metalness);

    
    float3 Lo = toEye;
    
    for (int i = 0; i < MaxLights; ++i)
    {
        float3 Li = -gLights[i].Direction;
    
        float3 Lh = normalize(Li + Lo);
        
        //TODO: Add radiance value to each light!
        float cosLi = max(0.0, dot(normal, Li));
        float cosLh = max(0.0, dot(normal, Lh));
        
        float3 diffuseBRDF = BRDF(mat.DiffuseAlbedo, Li, mat.FresnelR0, normal, metalness);
        float3 specularBRDF = BRDFSpecular(mat.DiffuseAlbedo, Li, mat.FresnelR0, pos, toEye, normal,
        cosLi, cosLo, cosLh, mat.Shininess);
        
        sum += (diffuseBRDF + specularBRDF) * gLights[i].Radiance * cosLi * dW;
    }
    
    float3 ambientLighting = IBLAmbient(mat, irradiance, specularIrradiance, specularBRDF, normal, cosLo);
    
    return sum + ambientLighting;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for directional lights.
//---------------------------------------------------------------------------------------
float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for point lights.
//---------------------------------------------------------------------------------------
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if(d > L.FalloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if(d > L.FalloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    // Scale by spotlight
    float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float4 ComputeLighting(Light gLights[MaxLights], Material mat,
                       float3 pos, float3 normal, float3 toEye,
                       float3 shadowFactor)
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for(i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], mat, normal, toEye);
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS+NUM_POINT_LIGHTS; ++i)
    {
        result += ComputePointLight(gLights[i], mat, pos, normal, toEye);
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}


