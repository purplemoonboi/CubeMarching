#include "MarchingCubeData.hlsli"
#include "GPUDataStructures.hlsli"

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
#define BIT_KEY_SIZE 4
#define BUCKET_SIZE 16
#define NUM_BUCKETS 32
#define SMX_SIZE_FERMI 32
#define SMX_SIZE_ATI 64
#define BLOCK_SIZE 512
#define GROUP_SIZE 32


StructuredBuffer<int> TriangulationTable        : register(t0);
Texture3D<float> DensityTexture                 : register(t1);

RWStructuredBuffer<Triangle> TriangleBuffer     : register(u0);
RWStructuredBuffer<uint> HistoPyramid           : register(u1);
RWStructuredBuffer<uint> gInputMortons          : register(u2);
RWStructuredBuffer<uint> gSortedMortons         : register(u3);
RWStructuredBuffer<uint> gBucketBuffer          : register(u4);
RWStructuredBuffer<int> gCycleCounter           : register(u5);
RWStructuredBuffer<uint> gVoxelConfigurations   : register(u6);


[numthreads(BLOCK_SIZE, 1, 1)]
void ConfigureVoxels(
    uint3 gId : SV_GroupID,
        uint3 gtId : SV_GroupThreadID,
            uint3 tId : SV_DispatchThreadID
)
{
    float3 ftId = float3(tId.xyz);

    ftId /= TextureSize - 1;
    gInputMortons[tId.x] = Morton3D(ftId.x, ftId.y, ftId.z);
    

    int3 cornerCoords[8];
    cornerCoords[0] = tId + int3(0, 0, 0);
    cornerCoords[1] = tId + int3(1, 0, 0);
    cornerCoords[2] = tId + int3(1, 0, 1);
    cornerCoords[3] = tId + int3(0, 0, 1);
    cornerCoords[4] = tId + int3(0, 1, 0);
    cornerCoords[5] = tId + int3(1, 1, 0);
    cornerCoords[6] = tId + int3(1, 1, 1);
    cornerCoords[7] = tId + int3(0, 1, 1);

    uint i = 0;
    uint cubeConfiguration = 0;
    for (i = 0; i < 8; i++)
    {
        if (DensityTexture[cornerCoords[i]] < 0)
        {
            cubeConfiguration |= (1 << i);
        }
    }
    
    gVoxelConfigurations[tId.x] = cubeConfiguration;
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

groupshared uint lInputCodes[BLOCK_SIZE];
groupshared uint lSortedCodes[BLOCK_SIZE];
groupshared uint lSums[BLOCK_SIZE];
groupshared uint lBits[BLOCK_SIZE];

#ifndef USING_NVIDIA_FERMI
#define USING_NVIDIA_FERMI
groupshared uint lBitBuckets[BLOCK_SIZE/SMX_SIZE_FERMI];
#else
groupshared uint lBuckets[BLOCK_SIZE / SMX_SIZE_ATI];
#endif

#define BLOCK_SIZE_F 512.0

[numthreads(BLOCK_SIZE, 1, 1)]
void LocalSort4BitKey(
        uint3 gId : SV_GroupID,
            uint3 gtId : SV_GroupThreadID,
                uint3 dtId : SV_DispatchThreadID,
                   uint gIdx : SV_GroupIndex
)
{  
    // #1 Each block sorts in local memory and computes 
    //    offsets for the 16 buckets.
        
#ifdef USING_NVIDIA_FERMI
#define USING_NVIDIA_FERMI
    const uint k = BLOCK_SIZE / SMX_SIZE_FERMI; //16    
#else
    uint k = BLOCK_SIZE / SMX_SIZE_ATI; //8
#endif
    
    // store the global input array into group 
    // shared memory.
    lInputCodes[dtId.x] = gInputMortons[(gId.x * BLOCK_SIZE) + dtId.x];
    // tracks the global cycle iteration.
    uint cycle = gCycleCounter[0];
    
    // @brief - while we have 512 threads in group memory, on Fermi cards
    // 32 threads will execute instructions on a SMX core (processing unit).
    // so we split the group into local groups offset with respect to 
    // (DispatchThread / BLOCK_SIZE) * NUM_THREADS 
    // which yields '16' groups for Fermi cards. (8 groups on ATI cards).
    
    // returns which sub-group this thread belongs to.
    float dtfId = dtId.x;
    uint lgId = (dtfId / BLOCK_SIZE_F) * k; // [0 - 16]
    
    // local group begin index
    uint lgBegin = (lgId * GROUP_SIZE);
    
    // local group end index.
    uint lgEnd = (lgId * GROUP_SIZE) + (GROUP_SIZE - 1);
    
    GroupMemoryBarrierWithGroupSync();
    uint dest = 0;

    uint mask = 1 << 0;
    uint code = lInputCodes[dtId.x];
    
    //...check the ith bit in the key...
    lBits[dtId.x] = (code & mask) ? 1 : 0;

    //...for our local group, perform a scan...
    for (uint t = lgBegin + 1; t < lgEnd; t = t * 2)
    {
        if(dtId.x >= t)
        {
            lSums[dtId.x] += lBits[dtId.x - t];
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    //uint falseTotal = BLOCK_SIZE - (lSums[(lgId * GROUP_SIZE) + GROUP_SIZE] + 1);
    uint falseTotal = GROUP_SIZE - (lSums[lgEnd] + lBits[lgEnd]);
    
    dest = lBits[dtId.x] ? lSums[dtId.x] + falseTotal : dtId.x - lSums[dtId.x];

    //...then scatter the code into group memory...
    uint sortedCode = lInputCodes[dtId.x];
    GroupMemoryBarrierWithGroupSync();
    
    lSortedCodes[dest] = sortedCode;

    //lSums[dtId.x] = 0;

    gSortedMortons[dtId.x] = lSortedCodes[dtId.x];
}



[numthreads(BLOCK_SIZE, 1, 1)]
void GlobalBucketSum(
    uint3 gId : SV_GroupID,
    uint3 gtId : SV_GroupThreadID,
    uint3 dtId : SV_DispatchThreadID
)
{
    
    // #1. save the cube configuration into the array
    //HPSums[dtId.x] = 0;
    //GroupMemoryBarrierWithGroupSync();
    
    //// #2. parallel prefix-sum 
    //uint i;
    //for (i = 1; i < X; i = i * 2)
    //{
    //    if (tId.x >= i)
    //    {
    //        HPSums[(tId.z * Z) * X + (tId.y * Y) + tId.x] +=
    //            HPSums[(tId.z * Z) * X + (tId.y * Y) + (tId.x - i)];
    //    }
    //    GroupMemoryBarrierWithGroupSync();
    //}
    
  
}

//[numthreads(32, 1, 1)]
//void ConstructLBVH(
//        uint3 gId : SV_GroupID, 
//            uint3 gtId : SV_GroupThreadID, 
//                uint3 dtId : SV_DispatchThreadID
//)
//{
    
    
    
//}
