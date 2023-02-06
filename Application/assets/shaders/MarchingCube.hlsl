
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


//TODO:CHANGE ME BACK TO TEX3D
Texture3D<float> DensityTexture : register(t0);
AppendStructuredBuffer<Triangle> triangles : register(u0);


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
        if (cellDensity[i] < isoLevel)
        {
            caseNumber |= 1 << i;
        }
    }


    if(edgeTable[caseNumber]== 0)
        return;

    if(caseNumber >= 128)
        return;

    float3 vertexList[12];
    float3 tangentList[12];

	if(edgeTable[caseNumber] & 1)
    {
        vertexList[0] = interpolation(corners[0], cellDensity[0], corners[1], cellDensity[1]);
        tangentList[0] = normalize(cellDensity[1] - cellDensity[0]);
    }

    if(edgeTable[caseNumber] & 2)
    {
        vertexList[1] = interpolation(corners[1], cellDensity[1], corners[2], cellDensity[2]);
        tangentList[1] = normalize(cellDensity[2] - cellDensity[1]);
    }

    if(edgeTable[caseNumber] & 4)
    {
        vertexList[2] = interpolation(corners[2], cellDensity[2], corners[3], cellDensity[3]);
        tangentList[2] = normalize(cellDensity[3] - cellDensity[2]);
    }

    if (edgeTable[caseNumber] & 8)
    {
        vertexList[3] = interpolation(corners[3], cellDensity[3], corners[0], cellDensity[0]);
        tangentList[3] = normalize(cellDensity[3] - cellDensity[0]);
    }

    if (edgeTable[caseNumber] & 16)
    {
        vertexList[4] = interpolation(corners[4], cellDensity[4], corners[5], cellDensity[5]);
        tangentList[4] = normalize(cellDensity[5] - cellDensity[4]);
    }

    if (edgeTable[caseNumber] & 32)
    {
        vertexList[5] = interpolation(corners[5], cellDensity[5], corners[6], cellDensity[6]);
        tangentList[5] = normalize(cellDensity[6] - cellDensity[5]);
    }

    if (edgeTable[caseNumber] & 64)
    {
        vertexList[6] = interpolation(corners[6], cellDensity[6], corners[7], cellDensity[7]);
        tangentList[6] = normalize(cellDensity[7] - cellDensity[6]);
    }

    if (edgeTable[caseNumber] & 128)
    {
        vertexList[7] = interpolation(corners[7], cellDensity[7], corners[4], cellDensity[4]);
        tangentList[7] = normalize(cellDensity[7] - cellDensity[4]);
    }

    if (edgeTable[caseNumber] & 256)
    {
        vertexList[8] = interpolation(corners[0], cellDensity[0], corners[4], cellDensity[4]);
        tangentList[8] = normalize(cellDensity[4] - cellDensity[0]);
    }

    if (edgeTable[caseNumber] & 512)
    {
        vertexList[9] = interpolation(corners[1], cellDensity[1], corners[5], cellDensity[5]);
        tangentList[9] = normalize(cellDensity[5] - cellDensity[1]);
    }

    if (edgeTable[caseNumber] & 1024)
    {
        vertexList[10] = interpolation(corners[2], cellDensity[2], corners[6], cellDensity[6]);
        tangentList[10] = normalize(cellDensity[6] - cellDensity[2]);
    }

    if (edgeTable[caseNumber] & 2048)
    {
        vertexList[11] = interpolation(corners[3], cellDensity[3], corners[7], cellDensity[7]);
        tangentList[11] = normalize(cellDensity[7] - cellDensity[3]);
    }


    Triangle tri = (Triangle)0;
    for (i = 0; triangulation[caseNumber][i] != -1; i+= 3)
    {
        int index = triangulation[caseNumber][i];
        tri.vertexA.position = vertexList[index];
        index = triangulation[caseNumber][i + 1];
        tri.vertexB.position = vertexList[index];
        index = triangulation[caseNumber][i + 2];
        tri.vertexC.position = vertexList[index];

        const float3 n0 = cross(normalize(tri.vertexA.position - tri.vertexB.position), normalize(tri.vertexA.position - tri.vertexC.position));
        const float3 n1 = cross(normalize(tri.vertexB.position - tri.vertexA.position), normalize(tri.vertexB.position - tri.vertexC.position));
        const float3 n2 = cross(normalize(tri.vertexC.position - tri.vertexA.position), normalize(tri.vertexC.position - tri.vertexB.position));

        tri.vertexA.normal = n0;
        tri.vertexB.normal = n1;
        tri.vertexC.normal = n2;

        tri.vertexA.id = threadId.xy;
        tri.vertexB.id = threadId.xy;
        tri.vertexC.id = threadId.xy;

        triangles.Append(tri);
    }


    //Triangle s;
    //s.vertexA.id = threadId.x;
    //s.vertexA.position = float3(0, 0, 0);
    //s.vertexA.normal = float3(1, 1, 1);
    //s.vertexB.id = threadId.y;
    //s.vertexB.position = float3(4, 4, 4);
    //s.vertexB.normal = float3(1, 1, 1);
    //s.vertexC.id = threadId.z;
    //s.vertexC.position = float3(8, 8, 8);
    //s.vertexC.normal = float3(1, 1, 1);

}



[numthreads(8, 8, 8)]
void GenerateChunkB(int3 threadId : SV_DispatchThreadID)
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
        if (cellDensity[i] < isoLevel)
        {
            caseNumber |= 1 << i;
        }
    }


    if (edgeTable[caseNumber] == 0)
        return;

    if (caseNumber < 128)
        return;

    float3 vertexList[12];
    float3 tangentList[12];

    if (edgeTable[caseNumber] & 1)
    {
        vertexList[0] = interpolation(corners[0], cellDensity[0], corners[1], cellDensity[1]);
        tangentList[0] = normalize(cellDensity[1] - cellDensity[0]);
    }

    if (edgeTable[caseNumber] & 2)
    {
        vertexList[1] = interpolation(corners[1], cellDensity[1], corners[2], cellDensity[2]);
        tangentList[1] = normalize(cellDensity[2] - cellDensity[1]);
    }

    if (edgeTable[caseNumber] & 4)
    {
        vertexList[2] = interpolation(corners[2], cellDensity[2], corners[3], cellDensity[3]);
        tangentList[2] = normalize(cellDensity[3] - cellDensity[2]);
    }

    if (edgeTable[caseNumber] & 8)
    {
        vertexList[3] = interpolation(corners[3], cellDensity[3], corners[0], cellDensity[0]);
        tangentList[3] = normalize(cellDensity[3] - cellDensity[0]);
    }

    if (edgeTable[caseNumber] & 16)
    {
        vertexList[4] = interpolation(corners[4], cellDensity[4], corners[5], cellDensity[5]);
        tangentList[4] = normalize(cellDensity[5] - cellDensity[4]);
    }

    if (edgeTable[caseNumber] & 32)
    {
        vertexList[5] = interpolation(corners[5], cellDensity[5], corners[6], cellDensity[6]);
        tangentList[5] = normalize(cellDensity[6] - cellDensity[5]);
    }

    if (edgeTable[caseNumber] & 64)
    {
        vertexList[6] = interpolation(corners[6], cellDensity[6], corners[7], cellDensity[7]);
        tangentList[6] = normalize(cellDensity[7] - cellDensity[6]);
    }

    if (edgeTable[caseNumber] & 128)
    {
        vertexList[7] = interpolation(corners[7], cellDensity[7], corners[4], cellDensity[4]);
        tangentList[7] = normalize(cellDensity[7] - cellDensity[4]);
    }

    if (edgeTable[caseNumber] & 256)
    {
        vertexList[8] = interpolation(corners[0], cellDensity[0], corners[4], cellDensity[4]);
        tangentList[8] = normalize(cellDensity[4] - cellDensity[0]);
    }

    if (edgeTable[caseNumber] & 512)
    {
        vertexList[9] = interpolation(corners[1], cellDensity[1], corners[5], cellDensity[5]);
        tangentList[9] = normalize(cellDensity[5] - cellDensity[1]);
    }

    if (edgeTable[caseNumber] & 1024)
    {
        vertexList[10] = interpolation(corners[2], cellDensity[2], corners[6], cellDensity[6]);
        tangentList[10] = normalize(cellDensity[6] - cellDensity[2]);
    }

    if (edgeTable[caseNumber] & 2048)
    {
        vertexList[11] = interpolation(corners[3], cellDensity[3], corners[7], cellDensity[7]);
        tangentList[11] = normalize(cellDensity[7] - cellDensity[3]);
    }


    Triangle tri = (Triangle) 0;
    for (i = 0; triangulationB[caseNumber - 128][i] != -1; i += 3)
    {
        int index = triangulationB[caseNumber - 128][i];
        tri.vertexA.position = vertexList[index];
        index = triangulationB[caseNumber - 128][i + 1];
        tri.vertexB.position = vertexList[index];
        index = triangulationB[caseNumber - 128][i + 2];
        tri.vertexC.position = vertexList[index];

        const float3 n0 = cross(normalize(tri.vertexA.position - tri.vertexB.position), normalize(tri.vertexA.position - tri.vertexC.position));
        const float3 n1 = cross(normalize(tri.vertexB.position - tri.vertexA.position), normalize(tri.vertexB.position - tri.vertexC.position));
        const float3 n2 = cross(normalize(tri.vertexC.position - tri.vertexA.position), normalize(tri.vertexC.position - tri.vertexB.position));

        tri.vertexA.normal = n0;
        tri.vertexB.normal = n1;
        tri.vertexC.normal = n2;

        tri.vertexA.id = threadId.xy;
        tri.vertexB.id = threadId.xy;
        tri.vertexC.id = threadId.xy;

        triangles.Append(tri);
    }


    //Triangle s;
    //s.vertexA.id = threadId.x;
    //s.vertexA.position = float3(0, 0, 0);
    //s.vertexA.normal = float3(1, 1, 1);
    //s.vertexB.id = threadId.y;
    //s.vertexB.position = float3(4, 4, 4);
    //s.vertexB.normal = float3(1, 1, 1);
    //s.vertexC.id = threadId.z;
    //s.vertexC.position = float3(8, 8, 8);
    //s.vertexC.normal = float3(1, 1, 1);

}