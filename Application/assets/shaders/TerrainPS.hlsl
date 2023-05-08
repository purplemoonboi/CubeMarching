#include "CoreUtils.hlsl"


struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoord : TEXCOORD;
};


float4 PS(VertexOut pin) : SV_TARGET
{
    
    float4 litColor = float4(0, 0, 0, 1);
    float4 ambient = (float4) 0;
    float4 directLight = (float4) 0;
    
    if (gWire != 0)
    {
        litColor = float4(0, 0, 0, 1);
    }
    else
    {
        // Interpolating normal can unnormalize it, so renormalize it.
        pin.NormalW = normalize(pin.NormalW);

        // Vector from point being lit to eye. 
        float3 toEyeW = normalize(gEyePosW - pin.PosW);
        
        float2 texCoords = (float2) 0;


        float shininess = 1.0f - gRoughness;
        Material mat = { float4(1, 1, 1, 1), gFresnelR0, gRoughness };

        float2 xUV = pin.PosW.zy / 16.0f;
        float2 yUV = pin.PosW.xz / 16.0f;
        float2 zUV = pin.PosW.xy / 16.0f;

        float4 xDiff  = gTextureMaps[3].Sample(PointWrapSampler, xUV);
        float3 xnDiff = gTextureMaps[4].Sample(PointWrapSampler, xUV).rgb;
            
        float4 yDiff  = gTextureMaps[1].Sample(PointWrapSampler, yUV);
        float3 ynDiff = gTextureMaps[2].Sample(PointWrapSampler, yUV).rgb;
            
        float4 zDiff  = gTextureMaps[3].Sample(PointWrapSampler, zUV);
        float3 znDiff = gTextureMaps[4].Sample(PointWrapSampler, zUV).rgb;
            
        float3 shadowFactor = 1.0f;
            
        xnDiff = NormalSampleToWorldSpace(xnDiff, pin.NormalW, pin.TangentW);
        ynDiff = NormalSampleToWorldSpace(ynDiff, pin.NormalW, pin.TangentW);
        znDiff = NormalSampleToWorldSpace(znDiff, pin.NormalW, pin.TangentW);

	    /* blinn phong */
        half3 blendWeights = pow(abs(pin.NormalW), 4);
        
        float3 blendedNormal = xnDiff * blendWeights.x + ynDiff * blendWeights.y + znDiff * blendWeights.z;
            
        blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);
            
        mat.DiffuseAlbedo = xDiff * blendWeights.x + yDiff * blendWeights.y + zDiff * blendWeights.z;
            
        ambient = gAmbientLight * xDiff * blendWeights.x + yDiff * blendWeights.y + zDiff * blendWeights.z;

        directLight = ComputeLighting(gLights, mat, pin.PosW, blendedNormal, toEyeW, shadowFactor);
            
        litColor = ambient + directLight;
    }

    
    return litColor;
}