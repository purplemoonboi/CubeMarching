//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif


// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

// Constant data that varies per frame.

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbMaterial : register(b1)
{
	float4 gDiffuseAlbedo;
    
    float3 gFresnelR0;
    float  gRoughness;
    
    float  gMetalness;
    float  gUseTexture;
    float  gUsePBR;
    float  gPad;
    
	float4x4 gMatTransform;
};

// Constant data that varies per material.
cbuffer cbPass : register(b2)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;

    float3 gEyePosW;
    float cbPerObjectPad1;
    
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;

    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    
    float4 gAmbientLight;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
    int gWire;
};
 
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

Texture2D Albedo : register(t0);
//Texture2D Normal : register(t1);
//Texture2D Roughness : register(t2);
//Texture2D AO : register(t3);

//TextureCube IrradienceTexture : register(t4);
//TextureCube SpecularTexture : register(t5);
//Texture2D SpecularBRDF : register(t6);

SamplerState DefaultSampler : register(s0);
//SamplerState SpecBRDFSampler : register(s1);


VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    vout.TangentW = mul(vin.Tangent, (float3x3) gWorld);

    vout.TexCoord = vin.TexCoord;
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

        Material mat = { gDiffuseAlbedo, gFresnelR0, gRoughness, gMetalness };

        float3 shadowFactor = 1.0f;

       /* blinn phong */
       
		// Indirect lighting.
      

        ambient = gAmbientLight * gDiffuseAlbedo;
    
       directLight = ComputeLighting(gLights, mat, pin.PosW,
		pin.NormalW, toEyeW, shadowFactor);

       litColor = ambient + directLight;

		// Common convention to take alpha from diffuse material.
       litColor.a = gDiffuseAlbedo.a;

        
		
    } 
    
    return litColor;
}


