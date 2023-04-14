#include "ComputeUtils.hlsli"


#define BIT_KEY_SIZE 30

#define BUCKET_SIZE 16
#define NUM_BUCKETS 16
#define SMX_SIZE_FERMI 32
#define GROUP_SIZE 32
#define BLOCK_SIZE 512
#define BLOCK_SIZE_F 512.0


RWStructuredBuffer<uint> gInputMortons  : register(u0);
RWStructuredBuffer<uint> gSortedMortons : register(u1);
RWStructuredBuffer<uint> gBucketBuffer  : register(u2);
RWStructuredBuffer<int>  gCycleCounter  : register(u3);

groupshared uint lLocalCodes[BLOCK_SIZE];
groupshared uint lSums[BLOCK_SIZE];
groupshared uint lBits[BLOCK_SIZE];
groupshared uint lDest[BLOCK_SIZE];
groupshared float falseTotals;


bool ExtractNBit(uint i, uint code)
{
    return ((code >> i) & 1) == 1;
}

[numthreads(BLOCK_SIZE, 1, 1)]
void LocalSort(
        //uint3 gId : SV_GroupID,
            uint3 gtId : SV_GroupThreadID,
                uint3 dtId : SV_DispatchThreadID,
                   uint gId : SV_GroupIndex
)
{


    // For debugging a keys frequence...
    // i do not trust myself that the radix sort
    // worked.
    //if(gId == 0)
    //{
    //    for (uint ff = 0; ff < 16;ff++)
    //    {
    //        lValOccurrence[ff] = 0;
    //    }
    //}

    //GroupMemoryBarrierWithGroupSync();


    //IncrimentKeyOccurence(lLocalCodes[gId]);


	// store the global input array into group 
    // shared memory.
    lLocalCodes[gId] = gInputMortons[gId];
    GroupMemoryBarrierWithGroupSync();

    // need to change this for morton codes.
    [unroll(BIT_KEY_SIZE)]
    for (uint i = 0; i < BIT_KEY_SIZE; i++)
    {
    
        //...check the ith bit in the key...
        lBits[gId] = ExtractNBit(i, lLocalCodes[gId]) == 0;
        GroupMemoryBarrierWithGroupSync();
        
        if (gId != 0)
        {
            lSums[gId] = lBits[gId - 1];
        }
        else
        {
            lSums[gId] = 0;
        }
        
        GroupMemoryBarrierWithGroupSync();

        //...for our local group, perform a prefix sum...
        [unroll(int(log2(BLOCK_SIZE)))]
        for (uint t = 1; t < BLOCK_SIZE; t<<= 1)//... for t = 1 ... log2(N), t = 2^t
        {
            uint tmp = 0;
            if (gId > t)
            {
                tmp = lSums[gId] + lSums[gId - t];

            }
            else
            {
                tmp = lSums[gId];

            }
            GroupMemoryBarrierWithGroupSync();

            lSums[gId] = tmp;
            GroupMemoryBarrierWithGroupSync();
        }

        
        if (gId == 0)
        {
            falseTotals =  (lSums[BLOCK_SIZE - 1] + lBits[BLOCK_SIZE - 1]);
        }
        GroupMemoryBarrierWithGroupSync();
    
        lDest[gId] = lBits[gId] ? lSums[gId] : gId - lSums[gId] + falseTotals;

        //...then scatter the code into group memory...
        uint sortedCode = lLocalCodes[gId];
        
        GroupMemoryBarrierWithGroupSync();

        lLocalCodes[lDest[gId]] = sortedCode;
        
        GroupMemoryBarrierWithGroupSync();

        //if (gId == 0)
        //{
        //    for (int a = 0; a < 16; a++)
        //    {
        //        gInputMortons[a] = lValOccurrence[a];
        //    }

        //}

    }

    gSortedMortons[gId] = lLocalCodes[gId];
}



[numthreads(BLOCK_SIZE, 1, 1)]
void GlobalBucketSum(
    uint3 gtId : SV_GroupThreadID,
    uint3 dtId : SV_DispatchThreadID,
	uint gId : SV_GroupIndex
)
{
    
   //...for our local group, perform a prefix sum...
	[unroll(int(log2(BLOCK_SIZE)))]
    for (uint t = 1; t < BLOCK_SIZE; t <<= 1)//... for t = 1 ... log2(N), t = 2^t
    {
        uint tmp = 0;
        if (gId > t)
        {
            tmp = lSums[gId] + lSums[gId - t];

        }
        else
        {
            tmp = lSums[gId];

        }
        GroupMemoryBarrierWithGroupSync();

        lSums[gId] = tmp;
        GroupMemoryBarrierWithGroupSync();
    }

    
  
}

[numthreads(BLOCK_SIZE, 1, 1)]
void GlobalDestination(
    uint3 gId : SV_GroupID,
    uint3 gtId : SV_GroupThreadID,
    uint3 dtId : SV_DispatchThreadID
)
{
    
    // #1. save the cube configuration into the array
    gBucketBuffer[dtId.x] = 0;
    GroupMemoryBarrierWithGroupSync();
    
    // #2. parallel prefix-sum 
    for (uint i = 1; i < BLOCK_SIZE; i = i * 2)
    {
        if (dtId.x >= i)
        {
            gBucketBuffer[dtId.x] += gBucketBuffer[dtId.x - i];
        }
        GroupMemoryBarrierWithGroupSync();
    }
    
  
}