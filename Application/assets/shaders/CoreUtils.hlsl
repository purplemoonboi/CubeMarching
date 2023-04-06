
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
    float4x4 gTexTransform;
    uint gMaterialIndex;
    uint gObjPad0;
    uint gObjPad1;
    uint gObjPad2;
};

cbuffer cbMaterial : register(b1)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;
    float4x4 gMatTransform;
    int DiffuseMapIndex;
    int NormalMapIndex;
    int RoughMapIndex;
    int AoMapIndex;
    int HeighMapIndex;
    uint gWire;
    uint2 Pad;
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
    Light gLights[MaxLights];
};

// space 0
Texture2D gTextureMaps[6]               : register(t0);
//TextureCube gCubeMap : register(t0);


// space 1


SamplerState PointWrapSampler           : register(s0);
SamplerState PointClampSampler          : register(s1);
SamplerState LinearWrapSampler          : register(s2);
SamplerState LinearClampSampler         : register(s3);
SamplerState AnisotropicWrapSampler     : register(s4);
SamplerState AnisotropicClampSampler    : register(s5);

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

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