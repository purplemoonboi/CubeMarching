
#define BIT_KEY_SIZE 30

#define NUM_BUCKETS 16
#define SMX_SIZE_FERMI 32
#define GROUP_SIZE 32
#define BLOCK_SIZE 512
#define BLOCK_SIZE_F 512.0

#define VOLUME16  16 * 16 * 16
#define VOLUME32  32 * 32 * 32
#define VOLUME64  64 * 64 * 64
#define VOLUME128 128 * 128 * 128

#define BUCKET_SIZE (32 * 32 * 32) / 512

cbuffer Volume : register(b0)
{
    int Width;
    int Height;
    int Depth;
}

RWStructuredBuffer<uint> gInputMortons  : register(u0);
RWStructuredBuffer<uint> gSortedMortons : register(u1);
RWStructuredBuffer<uint> gBucketBuffer  : register(u2);
RWStructuredBuffer<int>  gCycleCounter  : register(u3);

groupshared uint lLocalCodes[BLOCK_SIZE];
groupshared uint lSums[BLOCK_SIZE];
groupshared uint lBits[BLOCK_SIZE];
groupshared uint lDest[BLOCK_SIZE];
groupshared float falseTotals;

//  Binary & Hex - refresher
//  32-Bit Integer 
//
//
// H:| f    | 0    | 8    | 0    | 5    | c    | 0    | 1   
// B:| 1111 | 0000 | 0100 | 0000 | 0101 | 1100 | 0000 | 0001
// 
//


// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
uint ExpandBits(uint v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

//...we take a normalised 3D point
uint Morton3D(float x, float y, float z)
{
    x = min(max(x * Width, 0.0f), Width - 1);
    y = min(max(y * Height, 0.0f), Height - 1);
    z = min(max(z * Depth, 0.0f), Depth - 1);
    uint xx = ExpandBits((uint) x);
    uint yy = ExpandBits((uint) y);
    uint zz = ExpandBits((uint) z);
    return xx * 4 + yy * 2 + zz;
}



bool ExtractNBit(uint i, uint code)
{
    return ((code >> i) & 1) == 1;
}

[numthreads(BLOCK_SIZE,1,1)]
void EncodePoint(uint3 dtId : SV_DispatchThreadID)
{
    float z = (float)  dtId.x % Height;
    float y = ((float) dtId.x / Width) % Depth;
    float x = (float)  dtId.x / (Width * Depth);
    
    x /= Width;
    y /= Height;
    z /= Depth;
    
    gInputMortons[dtId.x] = Morton3D(x, y, z);
    
}

//...The local sort dispatch sorts a block of N-Bit keys
//...using a radix sort.
[numthreads(BLOCK_SIZE, 1, 1)]
void LocalSort(
        uint3 bId : SV_GroupID,
            uint3 gtId : SV_GroupThreadID,
                uint3 dtId : SV_DispatchThreadID,
                   uint gId : SV_GroupIndex
)
{

	
    uint cycle = gCycleCounter[0];
    
    // store the global input array into group 
    // shared memory.
    lLocalCodes[gId] = gInputMortons[gId];
    GroupMemoryBarrierWithGroupSync();

    //...for each bit
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

        //...prefix sum...
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

        if (gId == 0)
        {
            falseTotals = (lSums[BLOCK_SIZE - 1] + lBits[BLOCK_SIZE - 1]);
        }
        GroupMemoryBarrierWithGroupSync();
        
        //...split the local group with respect to the ith bit...
        lDest[gId] = lBits[gId] ? lSums[gId] : gId - lSums[gId] + falseTotals;

        uint sortedCode = lLocalCodes[gId];
        
        GroupMemoryBarrierWithGroupSync();
        lLocalCodes[lDest[gId]] = sortedCode;
        GroupMemoryBarrierWithGroupSync();
    }

    gInputMortons[dtId.x] = lLocalCodes[gId];
}




[numthreads(BLOCK_SIZE, 1, 1)]
void GlobalBucketSum(
    uint3 bId   : SV_GroupID,
    uint3 gtId  : SV_GroupThreadID,
    uint3 dtId  : SV_DispatchThreadID,
	uint  gId   : SV_GroupIndex
)
{
    
    
    
    [unroll(int(log2(BLOCK_SIZE)))]
    for (uint t = 1; t < BLOCK_SIZE; t <<= 1)
    {
        
        uint tmp = 0;
        if (gId > t)
        {
            tmp = gBucketBuffer[gId] + gBucketBuffer[gId - t];
        }
        else
        {
            tmp = gBucketBuffer[gId];
        }
        GroupMemoryBarrierWithGroupSync();
        gBucketBuffer[gId] = tmp;
        GroupMemoryBarrierWithGroupSync();    
    }
    
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
    lLocalCodes[gId] = gInputMortons[dtId.x];
    uint cycle = gCycleCounter[0];
    lBits[gId] = ExtractNBit(cycle, lLocalCodes[gId]) == 0;
    
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
    //... for t = 1 ... log2(N), t = 2^t
    [unroll(int(log2(BLOCK_SIZE)))]
    for (uint t = 1; t < BLOCK_SIZE; t <<= 1)
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
        falseTotals = (lSums[BLOCK_SIZE - 1] + lBits[BLOCK_SIZE - 1]);
           
    }
    GroupMemoryBarrierWithGroupSync();
        
    
    lDest[gId] = lBits[gId] ? lSums[gId] : gId - lSums[gId] + falseTotals;

        //...then scatter the code into group memory...
    uint sortedCode = lLocalCodes[gId];
        
    GroupMemoryBarrierWithGroupSync();
    lLocalCodes[lDest[gId]] = sortedCode;
    GroupMemoryBarrierWithGroupSync();
  
}