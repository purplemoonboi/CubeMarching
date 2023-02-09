
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
	int numPointsPerAxis;
	float3 chunkCoord;
};

cbuffer marchingCubeData : register(b1)
{
    int triangulation[256][16];
};

Texture3D<float> DensityTexture : register(t0);
AppendStructuredBuffer<Triangle> triangles : register(u0);

float3 interpolateVerts(float4 v1, float4 v2)
{
    float t = (isoLevel - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

int indexFromCoord(int x, int y, int z) {
    return z * numPointsPerAxis * numPointsPerAxis + y * numPointsPerAxis + x;
}

[numthreads(8, 8, 8)]
void GenerateChunk(int3 threadId : SV_DispatchThreadID)
{
 
    // Stop one point before the end because voxel includes neighbouring points
    if (threadId.x >= numPointsPerAxis - 1 || threadId.y >= numPointsPerAxis - 1 || threadId.z >= numPointsPerAxis - 1)
    {
        return;
    }

    // 8 corners of the current cube
    const float cubeCorners[8] =
    {
        DensityTexture[threadId.xyz],
        DensityTexture[float3(threadId.x + 1, threadId.y, threadId.z)],
        DensityTexture[float3(threadId.x + 1, threadId.y, threadId.z + 1)],
        DensityTexture[float3(threadId.x, threadId.y, threadId.z + 1)],
        DensityTexture[float3(threadId.x, threadId.y + 1, threadId.z)],
        DensityTexture[float3(threadId.x + 1, threadId.y + 1, threadId.z)],
        DensityTexture[float3(threadId.x + 1, threadId.y + 1, threadId.z + 1)],
        DensityTexture[float3(threadId.x, threadId.y + 1, threadId.z + 1)]
    };

    // Calculate unique index for each cube configuration.
    // There are 256 possible values
    // A value of 0 means cube is entirely inside surface; 255 entirely outside.
    // The value is used to look up the edge table, which indicates which edges of the cube are cut by the isosurface.
    int cubeIndex = 0;
    if (cubeCorners[0] < isoLevel)
        cubeIndex |= 1;
    if (cubeCorners[1] < isoLevel)
        cubeIndex |= 2;
    if (cubeCorners[2] < isoLevel)
        cubeIndex |= 4;
    if (cubeCorners[3] < isoLevel)
        cubeIndex |= 8;
    if (cubeCorners[4] < isoLevel)
        cubeIndex |= 16;
    if (cubeCorners[5] < isoLevel)
        cubeIndex |= 32;
    if (cubeCorners[6] < isoLevel)
        cubeIndex |= 64;
    if (cubeCorners[7] < isoLevel)
        cubeIndex |= 128;

    // Create triangles for current cube configuration
    //for (int i = 0; triangulation[cubeIndex][i] != -1; i += 3)
    //{
    //    // Get indices of corner points A and B for each of the three edges
    //    // of the cube that need to be joined to form the triangle.
    //    int a0 = cornerIndexAFromEdge[triangulation[cubeIndex][i]];
    //    int b0 = cornerIndexBFromEdge[triangulation[cubeIndex][i]];

    //    int a1 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 1]];
    //    int b1 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 1]];

    //    int a2 = cornerIndexAFromEdge[triangulation[cubeIndex][i + 2]];
    //    int b2 = cornerIndexBFromEdge[triangulation[cubeIndex][i + 2]];

    //    Triangle tri = (Triangle)0;
    //    //tri.vertexA.position = interpolateVerts(cubeCorners[a0], cubeCorners[b0]);
    //    //tri.vertexB.position = interpolateVerts(cubeCorners[a1], cubeCorners[b1]);
    //    //tri.vertexC.position = interpolateVerts(cubeCorners[a2], cubeCorners[b2]);

      
    //}
    Triangle tri = (Triangle) 0;

    tri.vertexA.position = float3(100, 100, 100);
    tri.vertexB.position = float3(200,200,200);
    tri.vertexC.position = float3(300,300,300);

   // triangles[threadId.x] = tri;
    triangles.Append(tri);

}

