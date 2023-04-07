#include "CoreUtils.hlsl"

 
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoord : TEXCOORD;
};



//TextureCube IrradienceTexture : register(t4);
//TextureCube SpecularTexture : register(t5);
//Texture2D SpecularBRDF : register(t6);




VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    vout.TangentW = mul(vin.Tangent, (float3x3) gWorld);

    float4 texC = mul(float4(vin.TexCoord, 0.0f, 1.0f), gTexTransform);
    vout.TexCoord = mul(texC, gMatTransform).xy;
    
    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 litColor = float4(0, 0, 0, 1);
    float4 ambient = (float4) 0;
    float4 directLight = (float4) 0;
    
    if (gWire == 0)
    {
       // Interpolating normal can unnormalize it, so renormalize it.
       pin.NormalW = normalize(pin.NormalW);

        	// Vector from point being lit to eye. 
       float3 toEyeW = normalize(gEyePosW - pin.PosW);

       float shininess = 1.0f - gRoughness;
       Material mat = { gDiffuseAlbedo, gFresnelR0, shininess };

       float3 shadowFactor = 1.0f;

		/* blinn phong */
       ambient = gAmbientLight * gDiffuseAlbedo;

       directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);
          
       litColor = ambient + directLight;

	   // Common convention to take alpha from diffuse material.
       litColor.a = gDiffuseAlbedo.a;

    } 
    
    return litColor;
}


