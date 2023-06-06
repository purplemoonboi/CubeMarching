#include "MarchingCubeData.hlsli"
#include "ComputeUtils.hlsli"

struct Vertex
{
    float3 Position;
    uint WorldIndex;
};


struct EdgeTableElement
{
    uint key;
    Vertex value;
};

struct Face
{
    uint i0;
    uint i1;
    uint i2;
};


cbuffer cbSettings : register(b0)
{
    float IsoLevel;
    int TextureSize;
    int UseBinarySearch;
    int NumPointsPerAxis;
    float3 ChunkCoord;
    int Resolution;
    int UseTexture;
};

Texture3D<float> DensityTexture : register(t0);
StructuredBuffer<int> TriangleTable : register(t1);

RWStructuredBuffer<Vertex> gVertices        : register(u0);
RWStructuredBuffer<Face> gFaces            : register(u1);
RWStructuredBuffer<int> gIndices                : register(u2);
RWStructuredBuffer<EdgeTableElement> gEdgeTable : register(u3);

#define EMPTY 0xffffffff
#define TABLE_SIZE16  4096
#define TABLE_SIZE32  32768
#define TABLE_SIZE64  262144
#define EDGE_COUNT 3
#define NUM_THREADS 256
#define CORNER_PER_VOXEL 8

//...we use a pair of vertices as the unique key
uint Hash(uint m, uint size)
{
    //...create the hash
    m ^= m >> 16;
    m *= 0x85ebca6b;
    m ^= m >> 13;
    m *= 0xc2b2ae35;
    m ^= m >> 16;
    return m & (size - 1);
}

//...add a new edge to the table.
uint AddValue(uint3 c0, uint3 c1, Vertex v, uint size)
{
    uint k = (c0.x + c1.x + c0.y + c1.y + c0.z + c1.z) % size;
    uint h = Hash(k, size);

    while(true)
    {
        uint p;
        //...atomic compare and swap. 
        InterlockedCompareExchange(gEdgeTable[h].key, EMPTY, k, p);
        
        //...if the value is unitialised
        if (p == EMPTY || p == k)
        {
            gEdgeTable[h].value = v;
            return h;
        }
        h = (h + 1) & (size - 1);
    }
    
    return h;
}

//...try to get the value with the provided key. 
Vertex GetValue(uint3 c0, uint3 c1, uint size)
{
    uint k = (c0.x + c1.x + c0.y + c1.y + c0.z + c1.z) % size;
    uint h = Hash(k, size);
    
    Vertex v;
    v.Position = (float3) 0;
    v.WorldIndex = EMPTY;
    
    while(true)
    {
        if(gEdgeTable[h].key == k)
        {
            return gEdgeTable[h].value;
        }
        if (gEdgeTable[h].key == EMPTY)
        {
            
            return v;
        }
        //..move index along buffer. will force a break when
        //..we reach the buffer size - 1.
        h = (h + 1) & (size - 1);
    }
   
    return v;
}

uint AddValue(uint i, Vertex v)
{
    gEdgeTable[i].value = v;
    return 1;
}

Vertex GetValue(uint i)
{
    return gEdgeTable[i].value;
}

void DeleteValue(uint3 c0, uint3 c1, uint size)
{
    uint k = (c0.x + c1.x + c0.y + c1.y + c0.z + c1.z) % size;
    uint h = Hash(k, size);
    
    while(true)
    {
        //...invalid key!
        if (gEdgeTable[h].key == EMPTY)
        {
            return;
        }
        if(gEdgeTable[h].key == k)
        {
            //...fill it with garbage!
            Vertex v;
            v.Position = (float3) 0;
            v.WorldIndex = EMPTY;
            gEdgeTable[h].value = v;
            gEdgeTable[h].key == EMPTY;
        }
        h = (h + 1) & (size - 1);
    }
}


[numthreads(1, 1, 1)]
void InitialiseHashTable(uint gId : SV_GroupIndex, uint3 dId : SV_DispatchThreadID)
{
    Vertex v = { (float3) 0, EMPTY };
    gEdgeTable[dId.x].key = EMPTY;
    gEdgeTable[dId.x].value = v;
}

groupshared uint    lSums[NUM_THREADS * EDGE_COUNT];
groupshared uint    lVertCounter[NUM_THREADS * EDGE_COUNT];
groupshared float3  lVertices[NUM_THREADS * EDGE_COUNT];

[numthreads(NUM_THREADS, 1, 1)]
void GenerateVertices(uint3 dId : SV_DispatchThreadID, uint gId : SV_GroupIndex,  uint3 bId : SV_GroupID, uint3 tId : SV_GroupThreadID)
{
    
    /*
    
        #1 - generate vertices along each edge
    
        #2 - generate vertex indices using prefix sum.
    
        #3 - calculate the number of faces in voxel
    
        #4 - compute face indices using prefix sum
    
        #5 - connect triangles together
    
    */
    
    float fRes = Resolution;
    float step = 1.0f / fRes;
    
    float3 p = IndexTo3DPoint(dId.x, Resolution, Resolution, Resolution);
    
    //...voxel centroid
    uint3 vc = uint3(p);

    int3 corner[3];
    corner[0] = vc + int3(1, 0, 0);
    corner[1] = vc + int3(0, 1, 0);
    corner[2] = vc + int3(0, 0, 1);

    float s0 = DensityTexture.Load(float4(vc, 0));
    
    //...offset index
    uint i = gId * 3;
    
    for (uint t = 0; t < 3; t++)
    {
        lVertCounter[i + t] = 0;
        lVertices[i + t] = (float3)0;
    }
    
    [unroll(EDGE_COUNT)]
    for (t = 0; t < EDGE_COUNT; t++)
    {
        Vertex v = { (float3) 0, EMPTY }; //GetValue(corner[c0[i]], corner[c1[i]], TABLE_SIZE32);
    //...if the vertex along this edge hasn't been calculated yet.....
    
        float s1 = DensityTexture.Load(float4(corner[t], 0));
        float s = 0.0f;
        float3 em = (float3) 0;
    
        v.WorldIndex = 0;
        v.Position = (float3) 0;

        if ((s0 > IsoLevel && s1 < IsoLevel))
        {
            s = (IsoLevel - s0) / (s1 - s0);
            em = (float3) vc + 0.5f * ((float3) corner[t] - (float3) vc);
                    
            v.Position = (float3) vc + s * ((float3) corner[t] - (float3) vc);
            v.WorldIndex = 1;
        
            lVertCounter[i + t] = 1;
            lVertices[i + t] = v.Position;
        
            //...using the corner coords, we generate a key from.
            AddValue(vc, corner[i], v, TABLE_SIZE32);
        }
        if ((s0 < IsoLevel && s1 > IsoLevel))
        {
            s = (IsoLevel - s1) / (s0 - s1);
            em = (float3) corner[t] + 0.5f * ((float3) vc - (float3) corner[t]);
        
            v.Position = (float3) corner[t] + s * ((float3) vc - (float3) corner[t]);
            v.WorldIndex = 1;
        
            lVertCounter[i + t] = 1;
            lVertices[i + t] = v.Position;
        
            //...using the corner coords, we generate a key from.
            AddValue(vc, corner[i], v, TABLE_SIZE32);
        }
    }
    GroupMemoryBarrierWithGroupSync();

    uint tmp = 0;
    uint off = 0;
    Vertex v, w;
    
    if (gId == 0)
    {
        lSums[gId] = 0;
    }
    else
    {
        lSums[gId] = lVertCounter[gId];
    }
    GroupMemoryBarrierWithGroupSync();
    
    //...sum the indices per thread
    [unroll(3)]
    for (uint f = i + 1; f < (i + 2); f++)
    {
        lSums[f] += lSums[f - 1];
    }
    GroupMemoryBarrierWithGroupSync();
    
    uint ltmp[3] = { 0, 0, 0 };
    //...prefix sum over the entire block
    [unroll(NUM_THREADS)]
    for (t = 1; t < NUM_THREADS; t = t * 3)
    {   
        if (i > t)
        {
            off = lSums[i - t];
            
            ltmp[0] = lSums[i] + off;              //
            ltmp[1] = lSums[i+1] + off;            // read buffer
            ltmp[2] = lSums[i+2] + off;            //
            GroupMemoryBarrierWithGroupSync();
            lSums[i] = ltmp[0];                    //
            lSums[i+1] = ltmp[1];                  // write to buffer
            lSums[i+2] = ltmp[2];                  //
            GroupMemoryBarrierWithGroupSync();

        }
    }
    
    gIndices[(dId.x * 3)] = lSums[i];
    gIndices[(dId.x * 3)+1] = lSums[i + 1];
    gIndices[(dId.x * 3)+2] = lSums[i + 2];
      
}

#define BLOCK_SIZE 512
#define GROUP_SIZE 64

[numthreads(BLOCK_SIZE, 1, 1)]
void GenerateIndices(uint3 dId : SV_DispatchThreadID, uint gId : SV_GroupIndex, uint3 bId : SV_GroupID, uint3 tId : SV_GroupThreadID)
{     
    uint tmp = 0;
    uint suf = 0;
    Vertex v, w;
    
    if(gId == 0)
    {
        lSums[gId] = 0;
    }
    else
    {
        lSums[gId] = gIndices[dId.x];
    }
    GroupMemoryBarrierWithGroupSync();
    
    //...local prefix sum 
    [unroll(int(log2(BLOCK_SIZE)))]
    for (uint t = 1; t < BLOCK_SIZE; t <<= 1)
    {
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
    
    
    if(gId == 0)
    {
        
    }
    
    //...for each vertex in subsequent blocks
    //...apply the offset from the previous blocks.
    [unroll(GROUP_SIZE)]
    for (t = 1; t < GROUP_SIZE; t++)
    {
        if (bId.x > t)
        {
            w.WorldIndex = gIndices[bId.x - t];
            
            [unroll(BLOCK_SIZE)]
            for (uint j = 0; j < BLOCK_SIZE; j++)
            {
                
                v = GetValue(bId.x + j);
        
                if (bId.x > t && v.WorldIndex != EMPTY)
                {
                    tmp = v.WorldIndex + w.WorldIndex;
                }
                
                GroupMemoryBarrierWithGroupSync();
                v.WorldIndex = tmp;
                AddValue(bId.x + j, v);
                GroupMemoryBarrierWithGroupSync();
                //...update this blocks highest index.
                if(j == BLOCK_SIZE - 1)
                {
                    gIndices[bId.x] = v.WorldIndex;
                }
            }
        }
    }
    
}

groupshared uint lFaceCount[NUM_THREADS];


[numthreads(NUM_THREADS, 1, 1)]
void GenerateFaces(uint3 dId : SV_DispatchThreadID, uint gId : SV_GroupIndex, uint3 bId : SV_GroupID, uint3 tId : SV_GroupThreadID)
{
    
    float fRes = Resolution;
    float step = 1.0f / fRes;
    float z = (float)  dId.x % fRes;
    float y = ((float) dId.x / fRes) % fRes;
    float x = (float)  dId.x / (fRes * fRes);
    
    uint3 vc = uint3(x, y, z);
    
    int3 coord = vc + int3(ChunkCoord);

    int3 corner[8];
    corner[0] = coord + int3(0, 0, 0);
    corner[1] = coord + int3(1, 0, 0);
    corner[2] = coord + int3(1, 0, 1);
    corner[3] = coord + int3(0, 0, 1);
    corner[4] = coord + int3(0, 1, 0);
    corner[5] = coord + int3(1, 1, 0);
    corner[6] = coord + int3(1, 1, 1);
    corner[7] = coord + int3(0, 1, 1);

    float s0 = 0.0f;
    float s1 = 0.0f;
    
    uint cube = 0x00;
    
    
    
    /*..calculate the number of faces in this voxel..*/
    [unroll(CORNER_PER_VOXEL)]
    for (uint i = 0; i < CORNER_PER_VOXEL; i++)
    {
        if (DensityTexture.Load(int4(corner[i], 0)) < IsoLevel)
        {
            cube |= (1 << i);
        }
    }
    
    uint fc = 0;
    if (!(cube & 255 || cube & 0))
    {
        [unroll(5)]
        for (i = 0; TriangleTable[(cube * 16) + i] != -1; i += 3)
        {
            fc += 1;
        }
    }
    lFaceCount[gId] = fc;   
    
    GroupMemoryBarrierWithGroupSync();
    
    //...sum the number of faces in this voxel...
    [unroll(int(log2(NUM_THREADS)))]
    for (uint t = 1; t < NUM_THREADS; t <<= 1)
    {
        uint tmp = 0;
        if (gId > t)
        {
            tmp = lFaceCount[gId] + lFaceCount[gId - t];
        }
        else
        {
            tmp = lFaceCount[gId];

        }
        GroupMemoryBarrierWithGroupSync();
        lFaceCount[gId] = tmp;
        GroupMemoryBarrierWithGroupSync();
    }

    
    
}

[numthreads(NUM_THREADS, 1, 1)]
void ConstructConnectivityInformation(uint3 dId : SV_DispatchThreadID, uint gId : SV_GroupIndex, uint3 bId : SV_GroupID, uint3 tId : SV_GroupThreadID)
{
    
}

[numthreads(NUM_THREADS, 1, 1)]
void DetectBoundaryVertices(uint3 dId : SV_DispatchThreadID, uint gId : SV_GroupIndex, uint3 bId : SV_GroupID, uint3 tId : SV_GroupThreadID)
{
    
}