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
    int UseBinarySearch;
    int NumPointsPerAxis;
	float3 ChunkCoord;
	int Resolution;
};

Texture3D<float> DensityTexture : register(t0);
StructuredBuffer<int> TriangleTable : register(t1);
RWStructuredBuffer<Triangle> triangles : register(u0);


float3 CalculateNormal(int3 coord)
{
    int3 offsetX = int3(1, 0, 0);
    int3 offsetY = int3(0, 1, 0);
    int3 offsetZ = int3(0, 0, 1);

    float dx = DensityTexture.Load(float4(coord + offsetX, 0)) - DensityTexture.Load(float4(coord - offsetX, 0));
    float dy = DensityTexture.Load(float4(coord + offsetY, 0)) - DensityTexture.Load(float4(coord - offsetY, 0));
    float dz = DensityTexture.Load(float4(coord + offsetZ, 0)) - DensityTexture.Load(float4(coord - offsetZ, 0));

    return normalize(float3(dx, dy, dz));
}


Vertex createVertex(int3 coordA, int3 coordB)
{

    float densityA = DensityTexture.Load(float4(coordA, 0));
    float densityB = DensityTexture.Load(float4(coordB, 0));

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
    int indexA = coordA.z * Resolution * Resolution + coordA.y * Resolution + coordA.x;
    int indexB = coordB.z * Resolution * Resolution + coordB.y * Resolution + coordB.x;

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
    if (id.x >= Resolution - 1 || id.y >= Resolution - 1 || id.z >= Resolution - 1)
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
	
 

    for (i = 0; i < 16 && TriangleTable[(cubeConfiguration * 16) + i] != -1; i += 3)
    {

        int a0 = cornerIndexAFromEdge[TriangleTable[(cubeConfiguration * 16) + i]];
        int a1 = cornerIndexBFromEdge[TriangleTable[(cubeConfiguration * 16) + i]];
        
        int b0 = cornerIndexAFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 1]];
        int b1 = cornerIndexBFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 1]];

        int c0 = cornerIndexAFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 2]];
        int c1 = cornerIndexBFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 2]];

		// Create triangle
        Triangle tri = (Triangle) 0;
        tri.vertexA = createVertex(cornerCoords[a0], cornerCoords[a1]);
        tri.vertexB = createVertex(cornerCoords[b0], cornerCoords[b1]);
        tri.vertexC = createVertex(cornerCoords[c0], cornerCoords[c1]);

        triangles[triangles.IncrementCounter()] = tri;
    }

}
