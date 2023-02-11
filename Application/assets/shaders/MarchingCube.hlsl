
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
    int textureSize;
	float planetSize;
	int pointsPerAxis;
	float3 chunkCoord;
};

cbuffer marchingCubeData : register(b1)
{
    int triangulation[256][16];
};

Texture3D<float> DensityTexture : register(t0);
AppendStructuredBuffer<Triangle> triangles : register(u0);

float3 interpolateVertex(float4 v1, float4 v2)
{
    float t = (isoLevel - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

int indexFromCoord(int x, int y, int z) {
    return z * pointsPerAxis * pointsPerAxis + y * pointsPerAxis + x;
}

[numthreads(8, 8, 8)]
void GenerateChunk(int3 threadId : SV_DispatchThreadID)
{
 
    if (threadId.x >= pointsPerAxis - 1 || threadId.y >= pointsPerAxis - 1 || threadId.z >= pointsPerAxis - 1)
    {
        return;
    }

    // 8 corners of the current cube
    const float4 cubeCorners[8] =
    {
        float4(threadId.xyz,                                              DensityTexture[threadId.xyz]),
        float4(float3(threadId.x + 1,   threadId.y,     threadId.z),      DensityTexture[int3(threadId.x + 1, threadId.y, threadId.z)]),
        float4(float3(threadId.x + 1,   threadId.y,     threadId.z + 1),  DensityTexture[int3(threadId.x + 1, threadId.y, threadId.z + 1)]),
        float4(float3(threadId.x,       threadId.y,     threadId.z + 1),  DensityTexture[int3(threadId.x, threadId.y, threadId.z + 1)]),
        float4(float3(threadId.x,       threadId.y + 1, threadId.z),      DensityTexture[int3(threadId.x, threadId.y + 1, threadId.z)]),
        float4(float3(threadId.x + 1,   threadId.y + 1, threadId.z),      DensityTexture[int3(threadId.x + 1, threadId.y + 1, threadId.z)]),
        float4(float3(threadId.x + 1,   threadId.y + 1, threadId.z + 1),  DensityTexture[int3(threadId.x + 1, threadId.y + 1, threadId.z + 1)]),
        float4(float3(threadId.x,       threadId.y + 1, threadId.z + 1),  DensityTexture[int3(threadId.x, threadId.y + 1, threadId.z + 1)])
    };

    // Calculate unique index for each cube configuration.
    // There are 256 possible values
    // A value of 0 means cube is entirely inside surface; 255 entirely outside.
    // The value is used to look up the edge table, which indicates which edges of the cube are cut by the isosurface.
    int cubeIndex = 0;
    if (cubeCorners[0].w < isoLevel)
        cubeIndex |= 1;
    if (cubeCorners[1].w < isoLevel)
        cubeIndex |= 2;
    if (cubeCorners[2].w < isoLevel)
        cubeIndex |= 4;
    if (cubeCorners[3].w < isoLevel)
        cubeIndex |= 8;
    if (cubeCorners[4].w < isoLevel)
        cubeIndex |= 16;
    if (cubeCorners[5].w < isoLevel)
        cubeIndex |= 32;
    if (cubeCorners[6].w < isoLevel)
        cubeIndex |= 64;
    if (cubeCorners[7].w < isoLevel)
        cubeIndex |= 128;

    // Create triangles for current cube configuration
    for (int i = 0; i < 16; i += 3)
    {
        if(triangulation[cubeIndex][i] == -1)
            break;

        // Get indices of corner points A and B for each of the three edges
        // of the cube that need to be joined to form the triangle.
        int a = triangulation[cubeIndex][i];
        int a0 = cornerIndexAFromEdge[a];
        int b0 = cornerIndexBFromEdge[a];

        int b = triangulation[cubeIndex][i + 1];
        int a1 = cornerIndexAFromEdge[b];
        int b1 = cornerIndexBFromEdge[b];

        int c = triangulation[cubeIndex][i + 2];
        int a2 = cornerIndexAFromEdge[c];
        int b2 = cornerIndexBFromEdge[c];

        Triangle tri = (Triangle) 0;
        //tri.vertexA.position = interpolateVertex(cubeCorners[a0], cubeCorners[b0]);
        //tri.vertexB.position = interpolateVertex(cubeCorners[a1], cubeCorners[b1]);
        //tri.vertexC.position = interpolateVertex(cubeCorners[a2], cubeCorners[b2]);

        tri.vertexA.position = float3(a0, b0, cubeIndex);
        tri.vertexB.position = float3(a1, b1, isoLevel);
        tri.vertexC.position = float3(a2, b2, pointsPerAxis);

        triangles.Append(tri);
      
    }

    // Test for read back

    //Triangle tri = (Triangle) 0;
    //tri.vertexA.position = float3(20, 20, 20);
    //tri.vertexB.position = float3(10, 10, 10);
    //tri.vertexC.position = (float3)threadId.xyz;
    //triangles[triangles.IncrementCounter()] = tri;

}

