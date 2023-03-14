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


float SampleDensity(int3 coord)
{
    coord = max(0, min(coord, TextureSize));
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


[numthreads(8, 8, 8)]
void GenerateVertices(int3 id : SV_DispatchThreadID, int3 gtid : SV_GroupThreadID)
{
    if (id.x >= Resolution - 1 || id.y >= Resolution - 1 || id.z >= Resolution - 1)
    {
        return;
    }

    uint threadId = (Resolution * id.y) + (Resolution * id.x) + id.z;
   
    int3 coord = id + int3(ChunkCoord);

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
    
    /* voxel is outwith iso threshold */
    if (cubeConfiguration == 0 || cubeConfiguration == 255)
    {
        Vertex vertex = (Vertex) 0;
        vertex.position = float3(999,999,999);
        vertex.normal = float3(gtid);
        vertex.configuration = 0;
        Vertices[threadId] = vertex;
        
        return;
    }
      
    const uint cornerIndexAFromEdge[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
    const uint cornerIndexBFromEdge[12] = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };

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
        int m1 = (cubeConfiguration >> c1) & 0xb1;
        int m2 = (cubeConfiguration >> c2) & 0xb1;
        
        /* if there is a sign change */
        if (!((m1 == 0 && m2 == 0) || (m1 == 1 && m2 == 1)))
        {
            /* ...calculate the intersecting vertex along the edge */
            float3 p1 = cornerCoords[c1];
            float3 p2 = cornerCoords[c2];
            
            float t = (IsoLevel - DensityTexture[p1]) / (DensityTexture[p2] - DensityTexture[p1]);
            float3 p = p1 + t * (p2 - p1);
            
            float3 n = CalculateNormal(p);
            
            QEF_Add(float4(n.x, n.y, n.z, 0), float4(p.x, p.y, p.z, 0), ATA, Atb, pointaccum, btb);
            averageNormal += n;
            avgPosition += p;
            edgeCount++;
        }
    }

    avgPosition /= edgeCount;
    
    averageNormal = normalize(averageNormal / edgeCount);
    float3 com = float3(pointaccum.x, pointaccum.y, pointaccum.z) / pointaccum.w;
    float4 solvedPosition = (float4) 0;
    
    float error = QEF_Solve(ATA, Atb, pointaccum, solvedPosition);
    float3 minimum = cornerCoords[0];
    float3 maximum = cornerCoords[0] + float3(1, 1, 1);

    /* sometimes the position generated spawns the vertex outside the voxel */
    /* if this happens place the vertex at the centre of mass */
    if (solvedPosition.x < minimum.x || solvedPosition.y < minimum.y || solvedPosition.z < minimum.z ||
        solvedPosition.x > maximum.x || solvedPosition.y > maximum.y || solvedPosition.z > maximum.z)
    {
        solvedPosition.xyz = com.xyz;
    }
    
        
    Vertex vertex = (Vertex) 0;
    
    //TODO: REMOVE THIS AN USE THE POS GENERATED FROM THE QEF
    
    vertex.position = solvedPosition;
    vertex.normal = float3(error,0,0);
    vertex.configuration = 1;
    
    uint index = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + id.x;

    Vertices[index] = vertex;
    
}


[numthreads(8,8,8)]
void GenerateTriangle(uint3 id : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
   
    if (
        id.x >= Resolution - 1 || id.y >= Resolution - 1 || id.z >= Resolution - 1
        )
    {
        return;
    }

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
    
    int3 right = int3(1, 0, 0);
    int3 up = int3(0, 1, 0);
    int3 forward = int3(0, 0, 1);
   
    /* we only want to check three times per voxel starting from the corner */
    int3 coord = id + int3(ChunkCoord);

   
    Triangle tri = (Triangle) 0;
    
    /* 'this' voxel */
    uint current = ((id.z * Resolution) *Resolution) + (id.y * Resolution) + id.x;
   
    /* along the z-axis */
    uint below_Z = ((id.z * Resolution) *Resolution) + ((id.y - 1) * Resolution) + id.x;
    
    uint below_left_Z = (((id.z - 1) * Resolution) * Resolution) + ((id.y - 1) * Resolution) + id.x;
    
    uint left_Z = (((id.z - 1) *Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    /* along the y-axis */
    uint left_Y = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    uint left_behind_Y = (((id.z - 1) *Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    uint behind_Y = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    /* along the x-axis */
    uint below_X = ((id.z * Resolution) *Resolution)  + ((id.y - 1) * Resolution) + id.x;

    uint below_behind_X = ((id.z * Resolution) * Resolution) + ((id.y - 1) * Resolution) + (id.x - 1);
    
    uint behind_X = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    
    
    /* check the x axes */
    if (
        ((DensityTexture[coord] > 0 && DensityTexture[right] < 0)
        ||
         (DensityTexture[coord] < 0 && DensityTexture[right] > 0))
        )
    {
        /* ...sweep around the current axes and append the vertices */
        if (Vertices[current].configuration == 1 && Vertices[below_X].configuration == 1 &&
            Vertices[below_behind_X].configuration == 1)
        {
            tri.vertexA = Vertices[below_X];
            tri.vertexB = Vertices[below_behind_X];
            tri.vertexC = Vertices[current];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[below_behind_X].configuration == 1 && Vertices[behind_X].configuration == 1 &&
            Vertices[current].configuration == 1)
        {
            tri.vertexA = Vertices[below_behind_X];
            tri.vertexB = Vertices[behind_X];
            tri.vertexC = Vertices[current];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
    }
   
    /* check the 'y' axis */
    if (
        (DensityTexture[coord] > 0 && DensityTexture[up] < 0)
        ||
         (DensityTexture[coord] < 0 && DensityTexture[up] > 0)
        )
    {
        /* ...sweep around the current axes and append the vertices */
        
        if (Vertices[current].configuration == 1 && Vertices[left_Y].configuration == 1 &&
            Vertices[left_behind_Y].configuration == 1)
        {
            tri.vertexA = Vertices[current];
            tri.vertexB = Vertices[left_Y];
            tri.vertexC = Vertices[left_behind_Y];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
       
        if (Vertices[left_behind_Y].configuration == 1 && Vertices[behind_Y].configuration == 1 &&
            Vertices[current].configuration == 1)
        {
            tri.vertexA = Vertices[left_behind_Y];
            tri.vertexB = Vertices[behind_Y];
            tri.vertexC = Vertices[current];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
          
    }
    
    /* check the forward axis */
    if (
        (DensityTexture[coord] > 0 && DensityTexture[forward] < 0)
        ||
         (DensityTexture[coord] < 0 && DensityTexture[forward] > 0)
        )
    {
        /* ...sweep around the current axes and append the vertices */
        if (Vertices[current].configuration == 1 && Vertices[left_Z].configuration == 1 &&
            Vertices[below_left_Z].configuration == 1)
        {
            tri.vertexA = Vertices[left_Z];
            tri.vertexB = Vertices[below_left_Z];
            tri.vertexC = Vertices[current];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[below_left_Z].configuration == 1 && Vertices[below_Z].configuration == 1 &&
            Vertices[current].configuration == 1)
        {
            tri.vertexA = Vertices[below_left_Z];
            tri.vertexB = Vertices[below_Z];
            tri.vertexC = Vertices[current];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }

}