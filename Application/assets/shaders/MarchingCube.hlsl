
#ifndef MARCHING_CUBES_DATA  
#define MARCHING_CUBES_DATA  
#include "MarchingCubeData.hlsli"
#endif
struct Vertex
{
	float3 position;
	float3 normal;
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
	float isoLevel;
	float planetSize;
	int textureSize;
	int numPointsPerAxis;
	float3 chunkCoord;
};

AppendStructuredBuffer<Triangle> triangles : register(u0);
Texture3D<float> DensityTexture : register(t0);

int indexFromCoord(int3 coord)
{
    coord = coord - int3(chunkCoord);
    return coord.z * numPointsPerAxis * numPointsPerAxis + coord.y * numPointsPerAxis + coord.x;
}

float sampleDensity(int3 coord)
{
    coord = max(0, min(coord, textureSize));
    return DensityTexture.Load(int4(coord, 0));
}

float3 calculateNormal(int3 coord)
{
    const int3 offsetX = int3(1, 0, 0);
    const int3 offsetY = int3(0, 1, 0);
    const int3 offsetZ = int3(0, 0, 1);

    float dx = sampleDensity(coord + offsetX) - sampleDensity(coord - offsetX);
    float dy = sampleDensity(coord + offsetY) - sampleDensity(coord - offsetY);
    float dz = sampleDensity(coord + offsetZ) - sampleDensity(coord - offsetZ);

    return normalize(float3(dx, dy, dz));
}

// Calculate the position of the vertex
// The position lies somewhere along the edge defined by the two corner points.
// Where exactly along the edge is determined by the values of each corner point.
Vertex createVertex(int3 coordA, int3 coordB)
{

    const float densityA = DensityTexture.Load(int4(coordA, 0));
    const float densityB = DensityTexture.Load(int4(coordB, 0));

	// Interpolate between the two corner points based on the density
    const float t = (isoLevel - densityA) / (densityB - densityA);
    const float3 position = coordA + t * (coordB - coordA);

	// Normal:
    const float3 normalA = calculateNormal(coordA);
    const float3 normalB = calculateNormal(coordB);
    const float3 normal = normalize(normalA + t * (normalB - normalA));

	// ID
    const int indexA = indexFromCoord(coordA);
    const int indexB = indexFromCoord(coordB);

	// Create vertex
    Vertex vertex;
    vertex.position = position;
    vertex.normal = normal;
    vertex.id = int2(min(indexA, indexB), max(indexA, indexB));
    return vertex;
}

float3 interpolation(float3 edgeVertex1, float valueAtVertex1, float3 edgeVertex2, float valueAtVertex2)
{
    return (edgeVertex1 + (isoLevel - valueAtVertex1) * (edgeVertex2 - edgeVertex1) / (valueAtVertex2 - valueAtVertex1));
}

[numthreads(8,8,8)]
void GenerateChunk(int3 threadId : SV_DispatchThreadID)
{   
    /* calculate the offsets of the corners using the thread id */
    float3 corners[8];
    corners[0] = float3(threadId);
    corners[1] = corners[0] + float3(0.f, 1.f, 0.f);
    corners[2] = corners[0] + float3(1.f, 1.f, 0.f);
    corners[3] = corners[0] + float3(1.f, 0.f, 0.f);

    corners[4] = corners[0] + float3(0.f, 0.f, 1.f);
    corners[5] = corners[0] + float3(0.f, 1.f, 1.f);
    corners[6] = corners[0] + float3(1.f, 1.f, 1.f);
    corners[7] = corners[0] + float3(1.f, 0.f, 1.f);

    float3 localCoords[8];
    float cellDensity[8];
    int caseNumber = 0;
    for (int i = 0; i < 8; i++)
    {
        localCoords[i] = corners[i];
        int3 index = int3(corners[i]);
        cellDensity[i] = DensityTexture.Load(int4(index, 0));
        if(cellDensity[i] < isoLevel)
        {
            caseNumber |= 1 << i;
        }
    }


    int edgeCase = edgeTable[caseNumber];

   // triangles.Append(tri);

}