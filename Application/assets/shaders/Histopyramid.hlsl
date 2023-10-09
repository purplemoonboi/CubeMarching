#include "MarchingCubeData.hlsli"
#include "ComputeUtils.hlsli"

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

struct Node
{
    int MortonCode;

    /* The 'Data' variable holds internal data of a node.
    * 
    * From the MSB....
    *
    * The first '8' parent node index.
    *
    * The next '8' bits represent number of child nodes.
    *
    * The next '8'  bits cube configuration 
    * (For MC algos this is the lookup index)
    * (For DC algos this can be a isValid flag)
    *
    * The remaining '8' bits are empty...
    * 
    */
    int Data;
};

cbuffer cbSettings : register(b0)
{
    float IsoLevel;
    int TextureSize;
    float PlanetSize;
    int Resolution;
    float3 ChunkCoord;
};

#define BLOCK_SIZE 512
#define MAX_DEPTH 5

StructuredBuffer<int> TriangulationTable            : register(t0);
Texture3D<float> DensityTexture                     : register(t1);

RWStructuredBuffer<Node> gLinearBoundVolumeHierarchy : register(u0);
RWStructuredBuffer<uint> gHistoPyramid               : register(u1);
RWStructuredBuffer<uint> gSortedMortons              : register(u2);

groupshared uint First;
groupshared uint Last;

Node NewLeafNode(int morton)
{
    Node node = { 0, 0 };
    
    return node;
}

uint Split(uint morton, uint first, uint last)
{
    
    //...Check MSB bit...
    uint split = first;
    uint step = last - first;
    uint commonPrefix = countbits(first ^ last);
    
    do
    {
        step = (step + 1) >> 1;
        uint newSplit = split + step;
        
        if(newSplit < last)
        {
            uint splitCode = gSortedMortons[newSplit];
            uint splitPrefix = countbits(First ^ splitCode);
            if(splitPrefix>commonPrefix)
            {
                split = newSplit;
            }
        }
        
    }
    while (step > 1);
    
    
    return split;
}

[numthreads(BLOCK_SIZE, 1, 1)]
void GenerateLBVH(
    uint3 bId : SV_GroupID,
        uint3 gtId : SV_GroupThreadID,
            uint3 tId : SV_DispatchThreadID,
                uint gId : SV_GroupIndex
)
{
    if(gId==0)
    {
        First = gSortedMortons[0];
        Last = gSortedMortons[Resolution-1];
    }
    
    GroupMemoryBarrierWithGroupSync();
    /*...Create masks for extrapolating data...*/
    int nodeMask = 1 >> 1;
    
    /*...Top Down Approach...*/
    //  1. - Find split in input data.
    //  Convert our thread ID into morton
    
    float z =  (float) gId  %  Resolution;
    float y = ((float) gId  /  Resolution) % Resolution;
    float x =  (float) gId  / (Resolution  * Resolution);
    
    uint morton = Morton3D(x,y,z, Resolution);
    
    //...now we can check the bits and split them 
    //...accordingly.
    uint split = Split(morton, First, Last);
    
    
}
