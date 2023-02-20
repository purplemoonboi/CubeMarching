// define the number of levels in the Histo-Pyramid
#define LEVELS 6

// define the dimensions of the voxel
float3 voxelSize = float3(1.0, 1.0, 1.0);

// define the threshold value for generating the surface
float isoValue = 0.5;

// define the vertex data structure
struct Vertex 
{
    float3 position;
    float3 normal;
    float3 tangent;
    float2 id;
};

// define the output data structure for the compute shader
struct Output {
    uint numVertices;
    Vertex vertices[MAX_VERTICES_PER_CUBE];
};

// define the 3D texture that contains the volumetric data
Texture3D<float> volumeTexture : register(t0);

// define the Histo-Pyramid texture
StructuredBuffer<uint> histoPyramid : register(t1);

// define the output buffer
RWStructuredBuffer<Output> outputBuffer : register(u0);

// compute shader for generating the surface using Histo-Pyramid Marching Cubes
[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
    uint3 cubeIndex = dispatchThreadId;
    uint3 cubeCornerIndex = cubeIndex * 2;
    uint level = 0;
    uint sum = 0;

    // calculate the sum of the values in the Histo-Pyramid up to the current level
    for (level = 0; level < LEVELS; level++) {
        uint histIndex = level * WIDTH * HEIGHT * DEPTH + cubeCornerIndex.z * WIDTH * HEIGHT + cubeCornerIndex.y * WIDTH + cubeCornerIndex.x;
        sum += histoPyramid[histIndex];
    }

    // skip the cube if it is completely inside or outside the surface
    if (sum == 0 || sum == 4096) {
        return;
    }

    // calculate the voxel corners for the current cube
    float3 p[8];
    p[0] = float3(cubeCornerIndex) * voxelSize;
    p[1] = p[0] + float3(voxelSize.x, 0.0, 0.0);
    p[2] = p[0] + float3(voxelSize.x, voxelSize.y, 0.0);
    p[3] = p[0] + float3(0.0, voxelSize.y, 0.0);
    p[4] = p[0] + float3(0.0, 0.0, voxelSize.z);
    p[5] = p[1] + float3(0.0, 0.0, voxelSize.z);
    p[6] = p[2] + float3(0.0, 0.0, voxelSize.z);
    p[7] = p[3] + float3(0.0, 0.0, voxelSize.z);

    // calculate the vertex data for the current cube
    Output output;
    output.numVertices = 0;

        // get the voxel values for the current cube
    float voxelValues[8];
    uint voxelIndex = cubeCornerIndex.z * WIDTH * HEIGHT + cubeCornerIndex.y * WIDTH + cubeCornerIndex.x;
    voxelValues[0] = volumeTexture.Load(int3(cubeCornerIndex)).r;
    voxelValues[1] = volumeTexture.Load(int3(cubeCornerIndex + uint3(1, 0, 0))).r;
    voxelValues[2] = volumeTexture.Load(int3(cubeCornerIndex + uint3(1, 1, 0))).r;
    voxelValues[3] = volumeTexture.Load(int3(cubeCornerIndex + uint3(0, 1, 0))).r;
    voxelValues[4] = volumeTexture.Load(int3(cubeCornerIndex + uint3(0, 0, 1))).r;
    voxelValues[5] = volumeTexture.Load(int3(cubeCornerIndex + uint3(1, 0, 1))).r;
    voxelValues[6] = volumeTexture.Load(int3(cubeCornerIndex + uint3(1, 1, 1))).r;
    voxelValues[7] = volumeTexture.Load(int3(cubeCornerIndex + uint3(0, 1, 1))).r;

    // calculate the vertex positions and normals for the current cube
    float3 vertexPositions[12];
    float3 vertexNormals[12];
    uint numVertices = 0;

    // calculate the vertex positions and normals for each triangle in the current cube
    for (uint i = 0; i < 5; i++) {
        uint edgeIndex = cubeIndex.z * WIDTH * HEIGHT * 3 + cubeIndex.y * WIDTH * 3 + cubeIndex.x * 3 + i * 2;
        uint edgeTableIndex = sum * 256 + edgeIndex;

        for (uint j = 0; j < 3; j++) {
            uint edge = edgeTable[edgeTableIndex * 3 + j];

            if (edge == 0) {
                break;
            }

            float3 p1 = p[edges[edge][0]];
            float3 p2 = p[edges[edge][1]];
            float v1 = voxelValues[edges[edge][0]];
            float v2 = voxelValues[edges[edge][1]];
            float t = (isoValue - v1) / (v2 - v1);

            vertexPositions[numVertices] = lerp(p1, p2, t);
            vertexNormals[numVertices] = getNormal(vertexPositions[numVertices]);

            numVertices++;
        }
    }

    // save the vertex data for the current cube to the output buffer
    output.numVertices = numVertices;
    for (uint i = 0; i < numVertices; i++) {
        output.vertices[i].position = vertexPositions[i];
    }
    outputBuffer[cubeIndex.y * WIDTH * DEPTH + cubeIndex.z * WIDTH + cubeIndex.x] = output;
}

