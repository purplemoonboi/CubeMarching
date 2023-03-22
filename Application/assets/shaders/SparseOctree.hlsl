#include "GPUDataStructures.hlsli"

#define MAX_LEVELS 10

struct Node 
{
  uint childPtr;
  uint leaf;
  uint count;
};

StructuredBuffer<Node> tree : register(t0);


/* sparse octree steps */

/* build octree bottom -> up evaluating each voxel */
