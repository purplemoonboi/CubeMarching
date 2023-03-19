#include "MarchingCubeData.hlsli"

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 id;
};

struct Triangle
{
    Vertex vertexC;
    Vertex vertexB;
    Vertex vertexA;
};

cbuffer cbSettings : register(b0)
{
    float IsoLevel;
    int TextureSize;
    float PlanetSize;
    int Resolution;
    float3 ChunkCoord;
};

Texture3D<float> DensityTexture : register(t0);
RWStructuredBuffer<Triangle> TriangleBuffer : register(u0);


RWTexture2D<int> HistoPyramidBase : register(u1);


[numthreads(1,1,1)]
void ConstructHP(int3 id : SV_DispatchThreadID)
{
    /* 
    *  First phase is building the histopyramid 
    *  from the base level to level 'k' 
    */    

    

}


[numthreads(1,1,1)]
void TraverseHP(int3 id : SV_DispatchThreadID)
{
    
}


