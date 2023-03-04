/* Simple QEF lib */
#include "QEF.hlsli"

struct DualVertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float3 voxelId;
    bool init = false;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float3 voxelId;
};

struct Triangle
{
    Vertex vertexA;
    Vertex vertexB;
    Vertex vertexC;
};

cbuffer cbSettings : register(b0)
{
    float isoLevel;
    int textureSize;
    float planetSize;
    int pointsPerAxis;
    float3 chunkCoord;
};

Texture3D<float> DensityTexture : register(t0);
StructuredBuffer<int> TriangleTable : register(t1);
RWStructuredBuffer<DualVertex> vertices : register(u0);
RWStructuredBuffer<Triangle> TriangleBuffer : register(u1);

float3 coordToWorld(int3 coord)
{
    return (coord / (textureSize - 1.0) - 0.5f) * planetSize;
}

int indexFromCoord(int3 coord)
{
    //coord = coord - int3(chunkCoord);
    return coord.z * pointsPerAxis * pointsPerAxis + coord.y * pointsPerAxis + coord.x;
}

float sampleDensity(int3 coord)
{
    coord = max(0, min(coord, textureSize));
    return DensityTexture.Load(float4(coord, 0));
}

float3 CalculateNormal(int3 coord)
{
    int3 offsetX = int3(1, 0, 0);
    int3 offsetY = int3(0, 1, 0);
    int3 offsetZ = int3(0, 0, 1);

    float dx = sampleDensity(coord + offsetX) - sampleDensity(coord - offsetX);
    float dy = sampleDensity(coord + offsetY) - sampleDensity(coord - offsetY);
    float dz = sampleDensity(coord + offsetZ) - sampleDensity(coord - offsetZ);

    return normalize(float3(dx, dy, dz));
}

/*
*  Linear interpolates between the two corners provided.
*  We want to approximate the position as a close to the zero surface.
*/
float3 ApproximateZeroCrossingPosition(float3 p0, float3 p1)
{
    float minValue = 100000.0f;
    float t = 0.0f;
    float currentT = 0.0f;
    float steps = 8;
    float incriment = 1.0f / steps;
    
    while(currentT <= 1.0f)
    {
        float3 p = p0 + ((p1 - p0) * currentT);
        float density = DensityTexture[p];
        if(density < minValue)
        {
            minValue = density;
            t = currentT;
        }
        
        currentT += incriment;
    }
    
    return p0 + ((p1 - p0) * t);
}


[numthreads(8, 8, 8)]
void GenerateChunk(int3 id : SV_DispatchThreadID)
{
    if (id.x >= pointsPerAxis - 1 || id.y >= pointsPerAxis - 1 || id.z >= pointsPerAxis - 1)
    {
        return;
    }
    
    int3 coord = id; // + int3(chunkCoord);

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
    if(cubeConfiguration == 0 || cubeConfiguration == 255)
        return;
    
  
    const uint cornerIndexAFromEdge[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
    const uint cornerIndexBFromEdge[12] = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };

   
    mat3x3_tri ATA = { 0, 0, 0, 0, 0, 0 };
    float4 pointaccum = (float4) 0;
    float4 Atb = (float4)0;
    float3 averageNormal = (float3) 0;
    float btb = (float) 0;
    float edgeCount = 0;
    
    for (i = 0; i < 12; i++)
    {
        /* fetch indices for the corners of the voxel */
        int c1 = cornerIndexAFromEdge[i];
        int c2 = cornerIndexBFromEdge[i];
        
        int m1 = (cubeConfiguration >> c1) & 1;
        int m2 = (cubeConfiguration >> c2) & 1;
        
        if (!((m1 == 0 && m2 == 0) || (m1 == 1 && m2 == 1)))
        {
            
            float3 p1 = cornerCoords[c1];
            float3 p2 = cornerCoords[c2];
            
            float3 p = ApproximateZeroCrossingPosition(p1, p2);
            float3 n = CalculateNormal(p);
            
            QEF_Add(float4(n.x, n.y, n.z, 0), float4(p.x, p.y, p.z, 0), ATA, Atb, pointaccum, btb);
            averageNormal += n;
            
            edgeCount++;
        }
    }
    
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
        
    DualVertex vertex = (DualVertex) 0;
    vertex.position = solvedPosition;
    vertex.normal = averageNormal;
    vertex.voxelId = id;
    vertex.tangent = float3(1, 0, 0);
    vertex.init = true;
    

    //vertices[vertices.IncrementCounter()] = vertex;
    vertices[(uint)id.xyz] = vertex;
}


[numthreads(4,4,4)]
void GenerateTriangle(uint3 did : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    
    /*
    *   In the second pass we're going to connect the vertices that were generated in 
    *   the first pass
    *   We need to check each edge for a sign change. This way we know for certain 
    *   a vertex was generated somewhere in the cell.
    */
    
    
    
    
}