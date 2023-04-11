#include "ComputeUtils.hlsli"


#define BIT_KEY_SIZE 4
#define BUCKET_SIZE 16
#define NUM_BUCKETS 32
#define SMX_SIZE_FERMI 32
#define SMX_SIZE_ATI 64
#define BLOCK_SIZE 32
#define GROUP_SIZE 32

RWStructuredBuffer<uint> gInputMortons  : register(u0);
RWStructuredBuffer<uint> gSortedMortons : register(u1);
RWStructuredBuffer<uint> gBucketBuffer  : register(u2);
RWStructuredBuffer<int>  gCycleCounter  : register(u3);

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

#define BLOCK_SIZE_F 32.0

[numthreads(2, 2, 2)]
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
    float3 dtfId = dtId;
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
    for (uint t = lgBegin; t < lgEnd; t++)
    {
        if (dtId.x >= t)
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
