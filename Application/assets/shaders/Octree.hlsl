#include "GPUDataStructures.hlsli"

#define OCTREE_MAX_DEPTH 4 // Levels [0 - 3]

struct Node 
{
  uint childPtr;
  uint leaf;
  uint count;
};

struct Voxel
{
    float3 position;
    uint index;
};

cbuffer VoxelWorldSettings : register(b0)
{
    float IsoLevel;
    int TextureSize;
    int UseBinarySearch;
    int NumPointsPerAxis;
    float3 ChunkCoord;
    int Resolution;
    int UseTexture;
}

RWStructuredBuffer<Node> Root : register(u0);// Contains all leaves.





RWStructuredBuffer<Voxel> VertexBuffer :register(u1);

Texture3D<float> DensityTexture : register(t0);
StructuredBuffer<int> TriangleTable : register(t1);

[numthreads(8,8,8)]
void ExtractIsosurface(
         uint3 gId : SV_GroupID,
            uint3 gtId : SV_GroupThreadID,
                uint3 dtId : SV_DispatchThreadID,
                   uint gIdx : SV_GroupIndex
)
{

    int3 coord = dtId; // +int3(ChunkCoord);

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
    
    if (cubeConfiguration == 0 || cubeConfiguration == 255)
        return;
	
    
    
}