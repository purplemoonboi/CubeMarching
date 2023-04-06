#include "GPUDataStructures.hlsli"

#define OCTREE_MAX_DEPTH 4 // Levels [0 - 3]

struct Node 
{
  uint childPtr;
  uint leaf;
  uint count;
};


RWStructuredBuffer<Node> Root : register(u0);// Contains all leaves.


[numthreads(8,8,8)]
void ConstructOctree(
         uint3 gId : SV_GroupID,
            uint3 gtId : SV_GroupThreadID,
                uint3 dtId : SV_DispatchThreadID,
                   uint gIdx : SV_GroupIndex
)
{

    
    
}