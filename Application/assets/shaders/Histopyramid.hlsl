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

cbuffer cbSettings : register(b0)
{
    float IsoLevel;
    int TextureSize;
    float PlanetSize;
    int Resolution;
    float3 ChunkCoord;
};

#define X 8
#define Y 8
#define Z 8



StructuredBuffer<int> TriangulationTable        : register(t0);
Texture3D<float> DensityTexture                 : register(t1);

RWStructuredBuffer<Triangle> TriangleBuffer     : register(u0);
RWStructuredBuffer<uint> HistoPyramid           : register(u1);

RWStructuredBuffer<uint> gVoxelConfigurations   : register(u6);


[numthreads(BLOCK_SIZE, 1, 1)]
void ConfigureVoxels(
    uint3 gId : SV_GroupID,
        uint3 gtId : SV_GroupThreadID,
            uint3 tId : SV_DispatchThreadID
)
{
    //float3 ftId = float3(tId.xyz);

    //ftId /= TextureSize - 1;
    //gInputMortons[tId.x] = Morton3D(ftId.x, ftId.y, ftId.z);
    

    //int3 cornerCoords[8];
    //cornerCoords[0] = tId + int3(0, 0, 0);
    //cornerCoords[1] = tId + int3(1, 0, 0);
    //cornerCoords[2] = tId + int3(1, 0, 1);
    //cornerCoords[3] = tId + int3(0, 0, 1);
    //cornerCoords[4] = tId + int3(0, 1, 0);
    //cornerCoords[5] = tId + int3(1, 1, 0);
    //cornerCoords[6] = tId + int3(1, 1, 1);
    //cornerCoords[7] = tId + int3(0, 1, 1);

    //uint i = 0;
    //uint cubeConfiguration = 0;
    //for (i = 0; i < 8; i++)
    //{
    //    if (DensityTexture[cornerCoords[i]] < 0)
    //    {
    //        cubeConfiguration |= (1 << i);
    //    }
    //}
    
    //gVoxelConfigurations[tId.x] = cubeConfiguration;
}

groupshared uint localSums[16];


[numthreads(BLOCK_SIZE,1,1)]
void PrefixSum(
    uint3 gId : SV_GroupID, 
        uint3 gtId : SV_GroupThreadID, 
            uint3 tId : SV_DispatchThreadID
)
{
    /* 
    *  First phase is building the histopyramid 
    *  from the base level to level 'k' 
    */  
    
   
    
    // #1. save the cube configuration into the array
    localSums[tId.x] = gVoxelConfigurations[tId.x];
    GroupMemoryBarrierWithGroupSync();
    
    // #2. parallel prefix-sum 
    uint i;

    for (i = 1; i < X; i = i * 2)
    {
        if (tId.x >= i)
        {
            localSums[tId.x] += (localSums[(tId.x - i)]);
        }
        GroupMemoryBarrierWithGroupSync();
    }
    
    // #3. Output
    HistoPyramid[tId.x] = localSums[tId.x];
    
}

groupshared uint Offsets[X * Y * Z];

[numthreads(X,Y,Z)]
void PrefixSumOffset(
    uint3 gId  : SV_GroupID,
    uint3 gtId : SV_GroupThreadID,
    uint3 tId  : SV_DispatchThreadID
)
{
   
    //  #1. Fetch totals of the previous segment
    if(tId.x < gId.x)
    {
        Offsets[(tId.z * Z) * X + (tId.y * Y) + tId.x] = HistoPyramid[((tId.z + 1) * Z) * X + (tId.y * Y) + tId.x];
    }
    else
    {
        Offsets[(tId.z * Z) * X + (tId.y * Y) + tId.x] = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    //  #2. Prefix sum 
    uint i;
    for (i = (X / 2); i > 0; i = (i / 2))
    {
        if(tId.x < i)
        {
            Offsets[(tId.z * Z) * X + (tId.y * Y) + tId.x] += 
                Offsets[(tId.z * Z) * X + (tId.y * Y) + (tId.x + i)];
        }
        GroupMemoryBarrierWithGroupSync();
    }
    //  #3. Output
    uint v0 = HistoPyramid[(tId.z * Z) * X + (tId.y * Y) + tId.x];
    HistoPyramid[(tId.z * Z) * X + (tId.y * Y) + tId.x] = Offsets[(tId.z * Z) * X + (tId.y * Y) + tId.x] + v0;
    
}

[numthreads(1, 1, 1)]
void TraverseHP(int3 id : SV_DispatchThreadID)
{
    
}


//#define THREAD_COUNT 512

[numthreads(X,Y,Z)]
void ComputeMortonCode(
        uint3 gId : SV_GroupID,
            uint3 gtId : SV_GroupThreadID,
                uint3 dtId : SV_DispatchThreadID
)
{
    // normalise the position into a unit cube of dimensions [0,1]
    float3 np = (float3) dtId.xyz / Resolution;
    gInputMortons[(dtId.z * Z) * X + dtId.y * Y + dtId.x] = Morton3D(np.x, np.y, np.z);
}

    
/*
    RWStructuredBuffer<uint> gInputMortons  : register(u2);
    RWStructuredBuffer<uint> gSortedMortons : register(u3);
    RWStructuredBuffer<uint> gBucketBuffer  : register(u4);
    RWStructuredBuffer<int>  gCycleCounter  : register(u5)
*/


