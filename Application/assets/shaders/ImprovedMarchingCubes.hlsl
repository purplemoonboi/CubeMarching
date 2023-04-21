#include "MarchingCubeData.hlsli"

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

RWStructuredBuffer<Vertex> gVertexBuffer        : register(u0);
RWStructuredBuffer<Face> gFaceBuffer            : register(u1);
RWStructuredBuffer<uint> gVertexCounter         : register(u2);
RWStructuredBuffer<EdgeTableElement> gEdgeTable : register(u3);

#define EMPTY 0xffffffff
#define TABLE_SIZE16  4096
#define TABLE_SIZE32  32768
#define TABLE_SIZE64  262144

#define NUM_THREADS 128
#define EDGE_COUNT 12
#define CORNER_PER_VOXEL 8

groupshared uint lFaceCount[NUM_THREADS];
groupshared uint lSums[NUM_THREADS * EDGE_COUNT];

groupshared uint lVertCounter[NUM_THREADS * EDGE_COUNT];
groupshared float3 lVertices[NUM_THREADS * EDGE_COUNT];
groupshared uint lEdges[NUM_THREADS * EDGE_COUNT];

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
    Vertex v = { (float3) 0, 0 };
    gEdgeTable[dId.x].key = EMPTY;
    gEdgeTable[dId.x].value = v;
}

[numthreads(NUM_THREADS, 1, 1)]
void GenerateVertices(uint3 dId : SV_DispatchThreadID, uint gId : SV_GroupIndex,  uint3 bId : SV_GroupID, uint3 tId : SV_GroupThreadID)
{
    
    //...Here is the overview of the improved algorithm
    //...Note: I don't expect this to be faster, but it should
    //...generate a tightly packed geometry, such that there are 
    //...no overlapping vertices at the voxel edges.
    /*
    
        #1 - generate vertices along each edge
    
        #2 - generate vertex indices using prefix sum.
    
        #3 - calculate the number of faces in voxel
    
        #4 - compute face indices using prefix sum
    
        #5 - connect triangles together
    
    */
    
    float fRes = Resolution;
    float step = 1.0f / fRes;
    
    float x = dId.x % fRes;
    float y = round(dId.x / (fRes * fRes));
    float z = round((dId.x / fRes) % fRes);
    
    uint3 vc = uint3(x, y, z);
    
    float3 p = (float3) 0;
    
    uint3 f = uint3(0, 0, 1);
    uint3 u = uint3(0, 1, 0);
    uint3 r = uint3(1, 0, 0);
    
    float s0 = DensityTexture.Load(float4(vc, 0));
    float s1 = 0.0f;
    
    //...zero out vertex indices
    [unroll(EDGE_COUNT)]
    for (uint i = 0; i < EDGE_COUNT; i++)
    {
        lVertCounter[gId + i] = 0;
        lVertices[gId + i] = (float3) 0;
    }

   
    int3 corner[8];
    corner[0] = vc + int3(0, 0, 0);
    corner[1] = vc + int3(1, 0, 0);
    corner[2] = vc + int3(1, 0, 1);
    corner[3] = vc + int3(0, 0, 1);
    corner[4] = vc + int3(0, 1, 0);
    corner[5] = vc + int3(1, 1, 0);
    corner[6] = vc + int3(1, 1, 1);
    corner[7] = vc + int3(0, 1, 1);
        
    uint c0[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
    uint c1[12] = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };
    
    uint isoMask = 0x10000000;//...intersects isosurface
    uint exMask  = 0x01000000;//...has already been indexed (additional check, is set through an atomic op)
    uint viMask  = ~(isoMask | exMask);//...mask for index value (normally 16-bits is fine to rep indices)
    
    [unroll(EDGE_COUNT)]
    for (i = 0; i < EDGE_COUNT; i++)
    {
        Vertex v = { (float3) 0, 0xffffffff }; //GetValue(corner[c0[i]], corner[c1[i]], TABLE_SIZE32);
        //...if the vertex along this edge hasn't been calculated yet.....
        
        float s0 = DensityTexture.Load(float4(corner[c0[i]], 0));
        float s1 = DensityTexture.Load(float4(corner[c1[i]], 0));
        float s = 0.0f;
        

        if ((s0 > IsoLevel && s1 < IsoLevel))
        {
            s = (IsoLevel - s0) / (s1 - s0);
            v.Position = (float3) corner[c0[i]] + s * ((float3)corner[c1[i]] - (float3)corner[c0[i]]);
            
            v.WorldIndex &= viMask; //...mark visited
            lVertCounter[(gId * EDGE_COUNT) + i] = v.WorldIndex;
            lVertices[(gId * EDGE_COUNT) + i] = v.Position;
            //...using the corner coords, we generate a key from.
            AddValue(corner[c0[i]], corner[c1[i]], v, TABLE_SIZE32);
        }
        if ((s0 < IsoLevel && s1 > IsoLevel))//...switch winding order s0'-' -> s1'+'
        {
            s = (IsoLevel - s1) / (s0 - s1);
            v.Position = (float3) corner[c1[i]] + s * ((float3) corner[c0[i]] - (float3)corner[c1[i]]);
            v.WorldIndex &= viMask; //...mark visited
            lVertCounter[(gId * EDGE_COUNT) + i] = v.WorldIndex;
            lVertices[(gId * EDGE_COUNT) + i] = v.Position;
            //...using the corner coords, we generate a key from.
            AddValue(corner[c0[i]], corner[c1[i]], v, TABLE_SIZE32);
        }

    }


    GroupMemoryBarrierWithGroupSync();
    uint tmp = 0;
    uint suf = 0;
    
    //...local prefix sum over voxel
    [unroll(EDGE_COUNT)]
    for (uint t = 1; t < EDGE_COUNT; t++)
    {
        Vertex v, w;
        v = GetValue(corner[c0[t]], corner[c1[t]], TABLE_SIZE32);
        w = GetValue(corner[c0[t - 1]], corner[c1[t - 1]], TABLE_SIZE32);
        
        uint val = ~viMask;
        
        if ((v.WorldIndex & (val & 0x1)) == 1 && (w.WorldIndex & (val & 0x1)) == 1)
        {
            if ((v.WorldIndex & (val & 0x01)) == 0)//  && (w.WorldIndex & (val & 0x01)) == 0)
            {
                tmp = (v.WorldIndex & viMask);
                suf = (w.WorldIndex & viMask);
                suf += tmp;
                //...update the vertex
                w.WorldIndex = (~viMask) | (suf & viMask);
                AddValue(corner[c0[t]], corner[c1[t]], v, TABLE_SIZE32);
                GroupMemoryBarrierWithGroupSync();
            }
        }
    }
    

    ////...prefix sum to generate vertex indices...
    //[unroll(int(log2(NUM_THREADS)))]
    //for (t = 1; t < NUM_THREADS; t <<= 1)
    //{
    //    if (gId > t)
    //    {
                
    //            //...check we have visited this edge already
    //        if (v.WorldIndex & viMask)
    //        {
    //                //...fetch it's index
    //            tmp = v.WorldIndex & isoMask;
                    
                    
    //        }
                
    //    }
    //    else
    //    {
    //        tmp = lSums[gId];
    //    }
    //    GroupMemoryBarrierWithGroupSync();
    //    lSums[gId] = tmp;
    //    GroupMemoryBarrierWithGroupSync();
    //}
    
    
    //...
    //...not sure if the perf is affected too much by this.
    //...didn't want global writes occurring back to back with
    //...global map insertions happening above.
    //...
    [unroll(EDGE_COUNT)]
    for (i = 0; i < EDGE_COUNT; i++)
    {
        Vertex v = GetValue(corner[c0[i]], corner[c1[i]], TABLE_SIZE32);
        //v.Position = lVertices[gId * EDGE_COUNT + i];
        //v.WorldIndex = lVertCounter[gId * EDGE_COUNT + i];
        gVertexBuffer[dId.x * EDGE_COUNT + i] = v;
        gVertexCounter[dId.x * EDGE_COUNT + i] = lVertCounter[gId * EDGE_COUNT + i];
    }
    
   
}

[numthreads(NUM_THREADS, 1, 1)]
void GenerateIndices(uint3 dId : SV_DispatchThreadID, uint gId : SV_GroupIndex, uint3 bId : SV_GroupID, uint3 tId : SV_GroupThreadID)
{
    
    lVertCounter[gId] = gVertexCounter[dId.x];
    GroupMemoryBarrierWithGroupSync();
    
    //...prefix sum to generate vertex indices...
    [unroll(int(log2(NUM_THREADS)))]
    for (uint t = 1; t < NUM_THREADS; t <<= 1)
    {
        uint tmp = 0;
        if (gId > t)
        {
            tmp = lVertCounter[gId] + lVertCounter[gId - t];
        }
        else
        {
            tmp = lVertCounter[gId];
        }
        GroupMemoryBarrierWithGroupSync();
        lSums[gId] = tmp;
        GroupMemoryBarrierWithGroupSync();
    }
    
}

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
        [unroll(15)]
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