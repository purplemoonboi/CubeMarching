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
        uint3 bId : SV_GroupID,
            uint3 gtId : SV_GroupThreadID,
                uint3 dtId : SV_DispatchThreadID,
                   uint gId : SV_GroupIndex
)
{

	// store the global input array into group 
    // shared memory.
    
    float z = (float)gId % 64.0f;
    float y = ((float)gId / 64.0f) % 64.0f;
    float x = (float)gId / (64.0f * 64.0f);
    
    uint trueIdx = Morton3D(x, y, z);
    uint cycle = gCycleCounter[0];
    GroupMemoryBarrierWithGroupSync();
    
    //lLocalCodes[gId] = gInputMortons[gId];
    lLocalCodes[gId] = trueIdx;
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
           if(cycle == i)
           {
                //...store the total active bits active w/ respect to the cycle.
                //this is to make the global scan, due next, easier.
                //gBucketBuffer[bId.x] = lSums[BLOCK_SIZE - 1] + lBits[bId.x*BLOCK_SIZE];
                gBucketBuffer[bId.x] = lSums[BLOCK_SIZE - 1];
            }
        }
        
       
        
        GroupMemoryBarrierWithGroupSync();
    
        lDest[gId] = lBits[gId] ? lSums[gId] : gId - lSums[gId] + falseTotals;

        //...then scatter the code into group memory...
        uint sortedCode = lLocalCodes[gId];
        
        GroupMemoryBarrierWithGroupSync();

        lLocalCodes[lDest[gId]] = sortedCode;
        
        GroupMemoryBarrierWithGroupSync();

    }

    gSortedMortons[gId] = lLocalCodes[gId];
}


groupshared uint lBuckets[BUCKET_SIZE];

[numthreads(BUCKET_SIZE, 1, 1)]
void GlobalBucketSum(
    uint3 bId   : SV_GroupID,
    uint3 gtId  : SV_GroupThreadID,
    uint3 dtId  : SV_DispatchThreadID,
	uint  gId   : SV_GroupIndex
)
{

    //...prefix sum over the global buckets...
	[unroll(int(log2(BUCKET_SIZE)))]
    for (uint t = 1; t < BUCKET_SIZE; t <<= 1)//... for t = 1 ... log2(N), t = 2^t
    {
        uint tmp = 0;
        
        if (gId > t)
        {
            tmp = lBuckets[gId] + lBuckets[gId - t];
        }
        else
        {
            tmp = lBuckets[gId];
        }
        GroupMemoryBarrierWithGroupSync();
        
        lBuckets[gId] = tmp;
        
        GroupMemoryBarrierWithGroupSync();
    }
    
    gBucketBuffer[gId] = lBuckets[gId];
    
    
}

[numthreads(BLOCK_SIZE, 1, 1)]
void GlobalDestination(
    uint3 bId   : SV_GroupID,
    uint3 gtId  : SV_GroupThreadID,
    uint3 dtId  : SV_DispatchThreadID,
    uint  gId   : SV_GroupIndex
)
{
    
    //...scatter keys back into the global array
    //...take the it's offset calculated in it's block 
    //...and add the offset from it's bucket.
    uint dest = lSums[gId] + gBucketBuffer[bId.x];
    
    uint code = gSortedMortons[gId];
    
    DeviceMemoryBarrierWithGroupSync();
    
    gInputMortons[dest] = code;
    
    
  
}