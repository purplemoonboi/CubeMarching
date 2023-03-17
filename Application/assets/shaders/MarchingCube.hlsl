#include "MarchingCubeData.hlsli"

struct Vertex
{
	float3 position;
	float3 normal;
    float3 tangent;
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
	float IsoLevel;
    int TextureSize;
	float PlanetSize;
	int Resolution;
	float3 ChunkCoord;
};

Texture3D<float> DensityTexture : register(t0);
StructuredBuffer<int> TriangleTable : register(t1);
RWStructuredBuffer<Triangle> triangles : register(u0);

float3 coordToWorld(int3 coord)
{
    return (coord / (TextureSize - 1.0) - 0.5f) * PlanetSize;
}

int indexFromCoord(int3 coord)
{
    //coord = coord - int3(ChunkCoord);
    return coord.z * Resolution * Resolution + coord.y * Resolution + coord.x;
}

float sampleDensity(int3 coord)
{
    coord = max(0, min(coord, TextureSize));
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

// Calculate the position of the vertex
// The position lies somewhere along the edge defined by the two corner points.
// Where exactly along the edge is determined by the values of each corner point.
Vertex createVertex(int3 coordA, int3 coordB)
{
	
    //float3 posA = coordToWorld(coordA);
    //float3 posB = coordToWorld(coordB);
    float densityA = sampleDensity(coordA);
    float densityB = sampleDensity(coordB);

	// Interpolate between the two corner points based on the density
    float t = (IsoLevel - densityA) / (densityB - densityA);
    float3 position = coordA + t * (coordB - coordA);

    position = position / (TextureSize - 1);
    position *= 32;
    
	// Normal:
    float3 normalA = CalculateNormal(coordA);
    float3 normalB = CalculateNormal(coordB);
    float3 normal = normalize(normalA + t * (normalB - normalA));

	// ID
    int indexA = indexFromCoord(coordA);
    int indexB = indexFromCoord(coordB);

	// Create vertex
    Vertex vertex;
    vertex.tangent = float3(1, 0, 0);
    vertex.position = position;
    vertex.normal = normal;
    vertex.id = int2(min(indexA, indexB), max(indexA, indexB));

    return vertex;
}

[numthreads(8, 8, 8)]
void GenerateChunk(int3 id : SV_DispatchThreadID)
{
    if (id.x >= Resolution || id.y >= Resolution  || id.z >= Resolution)
    {
        return;
    }

    
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

        if (DensityTexture[cornerCoords[i]] > IsoLevel)
        {
            cubeConfiguration |= (1 << i);
        }
    }
    
    if(cubeConfiguration == 0 || cubeConfiguration == 255)
        return;
	
    int edgeIndices[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

    for (int j = 0; j < 16; j++)
    {
        edgeIndices[j] = TriangleTable[(cubeConfiguration * 16) + j];
    }

    for (i = 0; i < 16; i += 3)
    {
        if (edgeIndices[i] == -1)
        {
            break;
        }

        int edgeIndexA = edgeIndices[i];
        int a0 = cornerIndexAFromEdge[edgeIndexA];
        int a1 = cornerIndexBFromEdge[edgeIndexA];

        int edgeIndexB = edgeIndices[i + 1];
        int b0 = cornerIndexAFromEdge[edgeIndexB];
        int b1 = cornerIndexBFromEdge[edgeIndexB];

        int edgeIndexC = edgeIndices[i + 2];
        int c0 = cornerIndexAFromEdge[edgeIndexC];
        int c1 = cornerIndexBFromEdge[edgeIndexC];

		// Create triangle
        Triangle tri = (Triangle) 0;
        tri.vertexA = createVertex(cornerCoords[a0], cornerCoords[a1]);
        tri.vertexB = createVertex(cornerCoords[b0], cornerCoords[b1]);
        tri.vertexC = createVertex(cornerCoords[c0], cornerCoords[c1]);

        triangles[triangles.IncrementCounter()] = tri;
    }

}
