/* Simple QEF lib */
#include "QEF.hlsli"

struct Vertex
{
    float3 position;
    float3 normal;
    int configuration;
};

struct Triangle
{
    Vertex vertexA;
    Vertex vertexB;
    Vertex vertexC;
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
RWStructuredBuffer<Vertex> Vertices : register(u0);
RWStructuredBuffer<Triangle> TriangleBuffer : register(u1);

RWStructuredBuffer<int> VoxelMaterialBuffer : register(u2);

// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
unsigned int ExpandBits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
unsigned int Morton3D(float x, float y, float z)
{
    x = min(max(x * 1024.0f, 0.0f), 1023.0f);
    y = min(max(y * 1024.0f, 0.0f), 1023.0f);
    z = min(max(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = ExpandBits((unsigned int) x);
    unsigned int yy = ExpandBits((unsigned int) y);
    unsigned int zz = ExpandBits((unsigned int) z);
    return xx * 4 + yy * 2 + zz;
}

float SampleDensity(int3 coord)
{
    //coord = max(0, min(coord, TextureSize));
    return DensityTexture.Load(float4(coord, 0));
    
}

float3 CalculateNormal(int3 coord)
{
    int3 offsetX = int3(1, 0, 0);
    int3 offsetY = int3(0, 1, 0);
    int3 offsetZ = int3(0, 0, 1);

    float dx = SampleDensity(coord + offsetX) - SampleDensity(coord - offsetX);
    float dy = SampleDensity(coord + offsetY) - SampleDensity(coord - offsetY);
    float dz = SampleDensity(coord + offsetZ) - SampleDensity(coord - offsetZ);

    return normalize(float3(dx, dy, dz));
}

static int2 EdgeMap[12] =
{
    int2(0, 4), int2(1, 5), int2(2, 6), int2(3, 7),
	int2(0, 2), int2(1, 3), int2(4, 6), int2(5, 7),
	int2(0, 1), int2(2, 3), int2(4, 5), int2(6, 7)
};



static const uint cornerIndexAFromEdge[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
static const uint cornerIndexBFromEdge[12] = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };



[numthreads(8, 8, 8)]
void GenerateVertices(int3 id : SV_DispatchThreadID, int3 gtid : SV_GroupThreadID)
{
    if (id.x >= Resolution - 1 || id.y >= Resolution - 1 || id.z >= Resolution - 1)
    {
        return;
    }

    uint index = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    int3 coord = id;// + int3(ChunkCoord);

    int3 cornerCoords[8];
    cornerCoords[0] = coord + int3(0, 0, 0);
    cornerCoords[1] = coord + int3(1, 0, 0);
    cornerCoords[2] = coord + int3(1, 0, 1);
    cornerCoords[3] = coord + int3(0, 0, 1);
    cornerCoords[4] = coord + int3(0, 1, 0);
    cornerCoords[5] = coord + int3(1, 1, 0);
    cornerCoords[6] = coord + int3(1, 1, 1);
    cornerCoords[7] = coord + int3(0, 1, 1);

    int cubeConfiguration = 0;
    for (int i = 0; i < 8; i++)
    {
        if (DensityTexture[cornerCoords[i]] > 0)
        {
            cubeConfiguration |= (1 << i);
        }
    }
    
    VoxelMaterialBuffer[index] = cubeConfiguration;
    
    /* voxel is outwith iso threshold */
    if (cubeConfiguration == 0 || cubeConfiguration == 255)
    {
        Vertex vertex = (Vertex) 0;
        vertex.position = float3(0,0,0);
        vertex.normal = id;
        vertex.configuration = -1;
        Vertices[index] = vertex;
        
        return;
    }
      
    mat3x3_tri ATA = { 0, 0, 0, 0, 0, 0 };
    float4 pointaccum = (float4) 0;
    float4 Atb = (float4) 0;
    float3 averageNormal = (float3) 0;
    float btb = (float) 0;
    float edgeCount = 0;
    
    /* for surface nets algo */
    float3 avgPosition = (float3) 0;
    
    for (i = 0; i < 12; i++)
    {
        /* fetch indices for the corners of the voxel */
        int c1 = cornerIndexAFromEdge[i];
        int c2 = cornerIndexBFromEdge[i];
        
        /* check the bit at this corner */
        //int m1 = (cubeConfiguration >> c1) & 0xb1;
        //int m2 = (cubeConfiguration >> c2) & 0xb1;
        
        float3 p1 = cornerCoords[c1];
        float3 p2 = cornerCoords[c2];
        
        int m1 = (DensityTexture[p1] < 0) ? 1 : 0;
        int m2 = (DensityTexture[p2] < 0) ? 1 : 0;
        
        /* if there is a sign change */
        if (!((m1 == 0 && m2 == 0) || (m1 == 1 && m2 == 1)))
        {
            /* ...calculate the intersecting vertex along the edge */
            p1 = cornerCoords[c1];
            p2 = cornerCoords[c2];
            
            float t = (IsoLevel - DensityTexture[p1]) / (DensityTexture[p2] - DensityTexture[p1]);
            float3 p = p1 + 0.5f * (p2 - p1);
            
            float3 n = CalculateNormal(p);
            //float3 normalA = CalculateNormal(p1);
            //float3 normalB = CalculateNormal(p2);
            //float3 n = normalize(normalA + t * (normalB - normalA));
            
            qef_add(float4(n.x, n.y, n.z, 0), float4(p.x, p.y, p.z, 0), ATA, Atb, pointaccum, btb);
            averageNormal += n;
            avgPosition += p;
            edgeCount++;
            
        }
    }

    avgPosition /= edgeCount;
    
    averageNormal = normalize(averageNormal / edgeCount);
    float3 com = float3(pointaccum.x, pointaccum.y, pointaccum.z) / pointaccum.w;
    float4 solvedPosition = (float4) 0;
    
    float error = qef_solve(ATA, Atb, pointaccum, solvedPosition);
    float3 minimum = cornerCoords[0];
    float3 maximum = cornerCoords[0] + float3(1, 1, 1);

    /* sometimes the position generated spawns the vertex outside the voxel */
    /* if this happens place the vertex at the centre of mass */
    if (solvedPosition.x < minimum.x || solvedPosition.y < minimum.y || solvedPosition.z < minimum.z ||
        solvedPosition.x > maximum.x || solvedPosition.y > maximum.y || solvedPosition.z > maximum.z)
    {
        solvedPosition.xyz = avgPosition;
    }
    
    if (avgPosition.x < minimum.x || avgPosition.y < minimum.y || avgPosition.z < minimum.z ||
        avgPosition.x > maximum.x || avgPosition.y > maximum.y || avgPosition.z > maximum.z)
    {
        avgPosition = id.xyz + float3(0.5f, 0.5f, 0.5f);
    }
   
    Vertex vertex = (Vertex) 0;
    
    //TODO: REMOVE THIS AN USE THE POS GENERATED FROM THE QEF
    
    vertex.position = avgPosition;
    vertex.normal = averageNormal;
    vertex.configuration = 1;
    
   
    
    //Vertices.IncrementCounter();
    Vertices[index] = vertex;
    
}


[numthreads(1,1,1)]
void GenerateTriangle(uint3 id : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
  

    
    
   /*   @brief
    *
    *   Each thread will work on an edge in parallel.
    *
    *   We check each direction again for a sign change. { right -> up -> forward }
    * 
    *   If there is a sign change along that edge...
    *   ...then we know the voxels parallel to the edge must contain vertices.
    *
    *   Using the voxel coord as the index into the vertex buffer... 
    *
    *   ...we append the vertices in anti-clockwise order to build the 
    *   triangle.
    *
    *   @note This is the naive approach and can be improved with Sparse octrees.
    */
    
    int3 right      = id + int3(1, 0, 0);
    int3 up         = id + int3(0, 1, 0);
    int3 forward    = id + int3(0, 0, 1);
    
   
    /* we only want to check three times per voxel starting from the corner */
    int3 coord = id ;//+ int3(ChunkCoord);

   
    Triangle tri = (Triangle) 0;
    
    /* 'this' voxel */
    uint pxyz = ((id.z * Resolution) *Resolution) + (id.y * Resolution) + id.x;

    if(VoxelMaterialBuffer[pxyz] == 0 || VoxelMaterialBuffer[pxyz] == 255)
    {
        return;
    }
    
    uint left_and_below_z = ((id.z * Resolution) * Resolution) + ((id.y - 1) * Resolution) + (id.x - 1);
    
    uint left_z = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    /* along the y-axis */
    uint left_of_y = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    uint left_and_behind_Y = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    uint behind_y = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    /* along the x-axis */
    uint below_pxyz = ((id.z * Resolution) * Resolution)  + ((id.y - 1) * Resolution) + id.x;

    uint right_of_and_below_x = (((id.z - 1)  * Resolution) * Resolution) + ((id.y - 1) * Resolution) + id.x;
    
    uint right_of_x = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + id.x;
   
    
    
    uint right_of_and_behind_y = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    
    /*
    *
    *   If the sign change is flipped i.e the edge extrudes from air into 
    *   solid from our perspective. Then build the triangles like so...
    *
    */
    
    /* check the z-axis */
    if (DensityTexture[coord] < 0 && DensityTexture[forward] > 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[pxyz].configuration == 1 && Vertices[left_z].configuration == 1 &&
            Vertices[left_and_below_z].configuration == 1)
        {
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_z];
            tri.vertexC = Vertices[left_and_below_z];
            
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[left_and_below_z].configuration == 1 && Vertices[below_pxyz].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_below_z];
            tri.vertexC = Vertices[below_pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
      
    if (DensityTexture[coord] > 0 && DensityTexture[forward] < 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[pxyz].configuration == 1 && Vertices[left_z].configuration == 1 &&
            Vertices[left_and_below_z].configuration == 1)
        {
        
            tri.vertexA = Vertices[left_and_below_z];
            tri.vertexB = Vertices[left_z];
            tri.vertexC = Vertices[pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[left_and_below_z].configuration == 1 && Vertices[below_pxyz].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[left_and_below_z];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[below_pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
    
    /* check the x-axes */
    if (DensityTexture[coord] < 0 && DensityTexture[right] > 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[pxyz].configuration == 1 && Vertices[below_pxyz].configuration == 1 &&
            Vertices[right_of_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[right_of_x];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[below_pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        
        if (Vertices[right_of_and_below_x].configuration == 1 && Vertices[right_of_and_below_x].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[right_of_x];
            tri.vertexB = Vertices[below_pxyz];
            tri.vertexC = Vertices[right_of_and_below_x];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
    if (DensityTexture[coord] > 0 && DensityTexture[right] < 0)
    {
         /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[pxyz].configuration == 1 && Vertices[right_of_x].configuration == 1 &&
            Vertices[right_of_and_below_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[below_pxyz];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[right_of_x];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[right_of_and_below_x].configuration == 1 && Vertices[below_pxyz].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[below_pxyz];
            tri.vertexB = Vertices[right_of_x];
            tri.vertexC = Vertices[right_of_and_below_x];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }

    /* check the y-axis */
    
    if (DensityTexture[coord] < 0 && DensityTexture[up] > 0)
    {
        
        if (Vertices[pxyz].configuration == 1 && Vertices[left_of_y].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1)
        {
        
        
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[right_of_x];
            tri.vertexC = Vertices[left_and_behind_Y];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
       
        if (Vertices[left_and_behind_Y].configuration == 1 && Vertices[behind_y].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {
       
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_behind_Y];
            tri.vertexC = Vertices[left_z];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
    
    if (DensityTexture[coord] > 0 && DensityTexture[up] < 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        
        if (Vertices[pxyz].configuration == 1 && Vertices[left_of_y].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1)
        {
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_of_y];
            tri.vertexC = Vertices[left_and_behind_Y];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
       
        if (Vertices[left_and_behind_Y].configuration == 1 && Vertices[behind_y].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {

            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_behind_Y];
            tri.vertexC = Vertices[behind_y];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    

 
    
}