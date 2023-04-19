#include "PerlinNoise.hlsli"

cbuffer cbPerlinSettings : register(b0)
{
    float Octaves;
    float Gain;
    float Loss;
    float Ground;
    
    float3 ChunkCoord;
    float Frequency;
    
    float Amplitude; //for heightmaps
    float BoundingMaxX;
    float BoundingMaxY;
    float BoundingMaxZ;
    

    int TextureWidth;
    int TextureHeight;
};

cbuffer cbCsgBuffer : register(b1)
{
    float MousePosX;
    float MousePosY;
    float MousePosZ;
    int IsMouseDown;
    float Radius;
    int DensityPrimitive;
    int CsgOperation;
    float deltaTime;
};


RWTexture3D<float> Noise3D : register(u0);
RWTexture3D<float> SecondaryNoise3D : register(u1);




[numthreads(1,1,1)]
void ComputeNoise3D(int3 id : SV_DispatchThreadID, int3 gId : SV_GroupID)
{
    
    float noise = 1;
    float freq = Frequency;
    float gain = Gain;
    float operation = 0.0f;
    float prim = 0.0f;
    float3 MousePos = float3(MousePosX, MousePosY, MousePosZ);
    
    float3 fId = (float3) id;//+ChunkCoord;
    
   
    
    if(IsMouseDown == 0)
    {
        for (int i = 0; i < Octaves; i++)
        {
            noise += snoise(fId * freq);
            freq *= gain;
        }
    }
    else
    {
        //fId /= TextureWidth - 1;
        //fId *= 16.0f;
        MousePos = abs(MousePos);
        switch (DensityPrimitive)
        {
            case 0:
                prim = Box(fId, MousePos/2.0f, Radius);
                break;
            case 1:
                prim = Sphere(fId, MousePos / 2.0f, Radius / 2.0f);
                break;
            case 2:
                prim = Cylinder(fId, MousePos / 2.0f, Radius);
                break;
            case 3:
                prim = Torus(fId, MousePos, Radius, Radius/4.0f);
                break;
        }
        switch (CsgOperation)
        {
            case 0:
                operation = -1.0f;
                break;
            case 1:
                operation = 1.0f;
                break;
        }
        
        noise = Noise3D[id];
        
        if(prim < Radius)
        {
            noise += operation;
            noise = clamp(noise, -1.0f, 1.0f);
        }
        
    }
    Noise3D[id] = noise;

    
}

[numthreads(8, 8, 8)]
void SmoothDensity(int3 id : SV_DispatchThreadID)
{
    if(id.x <= 0 || id.y <= 0 || id.z <= 0 ||
        id.x >= TextureWidth || id.y >= TextureHeight || id.z >= TextureWidth)
    {
        return;
    }
    
    float noise = 0.0f;
    
    noise += Noise3D[id];
    
    
    Noise3D[id] = noise;
}