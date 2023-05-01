#include "ComputeUtils.hlsli"


struct Node
{
    /*
    *   From MSB...
    *   First 15-bits   : Child Pointer
    *   Next  1-bit     : Far Pointer 
    *   Next  8-bits    : Valid Mask
    *   Last  8-bits    : Leaf Mask
    */
    uint Header;
    
    /*
    *   From MSB...
    *   First 24-bits   : Contour Pointer
    *   Last 8-bitd     : Contour Mask
    */
    uint ContourData;
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

RWStructuredBuffer<Node> Octree : register(u0);
RWStructuredBuffer<float3> VertexBuffer :register(u1);
Texture3D<float> DensityTexture : register(t0);

#define BLOCK_SIZE 512
#define OCTREE_MAX_DEPTH 4 // Levels [0 - 3]


[numthreads(BLOCK_SIZE,1,1)]
void BuildOctree(uint3 dId : SV_DispatchThreadID, uint gIdx : SV_GroupIndex)
{

    uint r = Resolution;
    
    float z = (float)  dId.x % r;
    float y = ((float) dId.x / r) % r;
    float x = (float)  dId.x / (r * r);
    
    uint Morton = Morton3D(x, y, z, Resolution);    
    
    uint ContourPointer = 0xffffff00;
    uint ContourMask    = ~ContourPointer;
    
    uint ChildPointer   = 0x000e0000;
    uint FarPointer     = 0x00010000;
    uint ValidMask      = 0x0000ff00;
    uint LeafMask       = 0x000000ff;
    
    Node vn = (Node) 0;

}