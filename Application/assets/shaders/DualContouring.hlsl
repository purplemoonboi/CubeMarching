#define NUM_THREADS_PER_GROUP 8
#define MAX_CUBES_PER_THREAD 8
#define MAX_EDGES_PER_CUBE 12

struct Vertex 
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 id;
};

struct Voxel 
{
    float density;
    bool active;
};

cbuffer WorldSettings : register(b0)
{
    uint3 gridSize;
    uint numVoxels;
    float isoLevel;
}

RWStructuredBuffer<Vertex> outputVertices : register(u0);

uint3 corners[8] = {
    uint3(0, 0, 0),
    uint3(1, 0, 0),
    uint3(1, 0, 1),
    uint3(0, 0, 1),
    uint3(0, 1, 0),
    uint3(1, 1, 0),
    uint3(1, 1, 1),
    uint3(0, 1, 1)
};

uint3 edges[12][2] = {
    { uint3(0, 0, 0), uint3(1, 0, 0) },
    { uint3(1, 0, 0), uint3(1, 0, 1) },
    { uint3(1, 0, 1), uint3(0, 0, 1) },
    { uint3(0, 0, 1), uint3(0, 0, 0) },
    { uint3(0, 1, 0), uint3(1, 1, 0) },
    { uint3(1, 1, 0), uint3(1, 1, 1) },
    { uint3(1, 1, 1), uint3(0, 1, 1) },
    { uint3(0, 1, 1), uint3(0, 1, 0) },
    { uint3(0, 0, 0), uint3(0, 1, 0) },
    { uint3(1, 0, 0), uint3(1, 1, 0) },
    { uint3(1, 0, 1), uint3(1, 1, 1) },
    { uint3(0, 0, 1), uint3(0, 1, 1) }
};

float3 calculateGradient(float3 position) 
{
    float3 gradient;

    gradient.x = calculateDensity(position + float3(epsilon, 0, 0)) - calculateDensity(position - float3(epsilon, 0, 0));
    gradient.y = calculateDensity(position + float3(0, epsilon, 0)) - calculateDensity(position - float3(0, epsilon, 0));
    gradient.z = calculateDensity(position + float3(0, 0, epsilon)) - calculateDensity(position - float3(0, 0, epsilon));

    return normalize(gradient);
}

float calculateDensity(float3 position) 
{
    return (position.y - 5) * (position.y - 5) + (position.x * position.x + position.z * position.z) * 0.5;
}

uint intersectEdge
(
    float densityA, float densityB, uint3 cornerA, uint3 cornerB, 
    out float3 edgePositions[2], out float3 edgeNormals[2]
) 
{
    if (densityA == densityB) 
    {
        return 0;
    }

    float3 positionA = cornerA;
    float3 positionB = cornerB;

    float mu = (isoLevel - densityA) / (densityB - densityA);
    float3 position = lerp(positionA, positionB, mu);
    edgePositions[0] = position;

    float3 normalA = calculateGradient(positionA);
    float3 normalB = calculateGradient(positionB);
    float3 normal = lerp(normalA, normalB, mu);

    edgeNormals[0] = normal;

    if (densityA > isoLevel) 
    {
        edgeNormals[0] *= -1.0f;
    }

    if (densityB > isoLevel) 
    {
        edgeNormals[0] *= -1.0f;
    }

    uint numVertices = 1;

    if (densityA > isoLevel)
     {
        float muA = (isoLevel - densityB) / (densityA - densityB);
        float3 positionA = lerp(positionB, positionA, muA);
        edgePositions[1] = positionA;

        float3 normalA = calculateGradient(positionA);
        edgeNormals[1] = normalA;

        if (densityA > isoLevel) {
            edgeNormals[1] *= -1.0f;
        }

        numVertices++;
    }

    if (densityB > isoLevel) 
    {
        float muB = (isoLevel - densityA) / (densityB - densityA);
        float3 positionB = lerp(positionA, positionB, muB);
        edgePositions[numVertices] = positionB;

        float3 normalB = calculateGradient(positionB);
        edgeNormals[numVertices] = normalB;

        if (densityB > isoLevel) {
            edgeNormals[numVertices] *= -1.0f;
        }

        numVertices++;
    }

    return numVertices;
}


[numthreads(NUM_THREADS_PER_GROUP, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID) 
{
    uint voxelIndex = groupID.x * NUM_THREADS_PER_GROUP + groupThreadID.x;
    if (voxelIndex >= numVoxels) {
        return;
    }

    uint3 voxelPosition = uint3(voxelIndex % gridSize.x, (voxelIndex / gridSize.x) % gridSize.y, voxelIndex / (gridSize.x * gridSize.y));
    uint cubeIndex = 0;
    Voxel voxels[8];
    for (uint i = 0; i < 8; i++) {
        uint3 cornerPosition = voxelPosition + corners[i];
        uint cornerIndex = cornerPosition.x + cornerPosition.y * (gridSize.x + 1) + cornerPosition.z * (gridSize.x + 1) * (gridSize.y + 1);
        voxels[i] = readVoxelFromBuffer(cornerIndex); // readVoxelFromBuffer is a function that reads a Voxel from a structured buffer
        if (voxels[i].density < isoLevel) {
            cubeIndex
            |= (1 << i);
        }
    }

    if (cubeIndex == 0 || cubeIndex == 255) {
        return;
    }

    float3 vertexPositions[MAX_EDGES_PER_CUBE];
    float3 vertexNormals[MAX_EDGES_PER_CUBE];
    uint numVertices = 0;

    for (uint i = 0; i < 12; i++) {
        if (cubeIndex & (1 << edges[i][0]) != 0) {
            if (cubeIndex & (1 << edges[i][1]) != 0) {
                float3 edgePositions[2];
                float3 edgeNormals[2];
                uint numEdgeVertices = intersectEdge(voxels[edges[i][0]].density, voxels[edges[i][1]].density, corners[edges[i][0]], corners[edges[i][1]], edgePositions, edgeNormals); // intersectEdge is a function that calculates the intersection of an edge with the iso-surface
                vertexPositions[numVertices] = edgePositions[0];
                vertexNormals[numVertices] = edgeNormals[0];
                numVertices++;
                if (numEdgeVertices == 2) {
                    vertexPositions[numVertices] = edgePositions[1];
                    vertexNormals[numVertices] = edgeNormals[1];
                    numVertices++;
                }
            }
        } else {
            if (cubeIndex & (1 << edges[i][1]) != 0) {
                float3 edgePositions[2];
                float3 edgeNormals[2];
                uint numEdgeVertices = intersectEdge(voxels[edges[i][1]].density, voxels[edges[i][0]].density, corners[edges[i][1]], corners[edges[i][0]], edgePositions, edgeNormals); // intersectEdge is a function that calculates the intersection of an edge with the iso-surface
                vertexPositions[numVertices] = edgePositions[0];
                vertexNormals[numVertices] = edgeNormals[0];
                numVertices++;
                if (numEdgeVertices == 2) {
                    vertexPositions[numVertices] = edgePositions[1];
                    vertexNormals[numVertices] = edgeNormals[1];
                    numVertices++;
                }
            }
        }
    }

    if (numVertices == 0) {
        return;
    }

    float3 positionSum = float3(0, 0, 0);
    float3 normalSum = float3(0, 0, 0);

    for (uint i = 0; i < numVertices; i++) {
        positionSum += vertexPositions[i];
        normalSum += vertexNormals[i];
    }

    float3 position = positionSum / numVertices;
    float3 normal = normalize(normalSum);

    uint outputIndex = InterlockedAdd(outputVerticesCounter, 1);
    outputVertices[outputIndex].position = position;
    outputVertices[outputIndex].normal = normal;
}


