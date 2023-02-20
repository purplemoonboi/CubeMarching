#define MAX_LEVELS 10

struct Node {
  uint childPtr : 30;
  uint leaf : 1;
  uint count : 1;
};

StructuredBuffer<Node> tree : register(t0);

uint3 EncodeMorton(uint3 position) {
  uint3 result;
  result.x = position.x | (position.x << 10);
  result.x = result.x & 0x924924;
  result.x = result.x | (result.x >> 5);
  result.x = result.x & 0x124924;

  result.y = position.y | (position.y << 10);
  result.y = result.y & 0x924924;
  result.y = result.y | (result.y >> 5);
  result.y = result.y & 0x124924;

  result.z = position.z | (position.z << 10);
  result.z = result.z & 0x924924;
  result.z = result.z | (result.z >> 5);
  result.z = result.z & 0x124924;

  return result;
}

uint EncodeMorton(uint3 position) {
  uint3 morton = EncodeMorton(position);
  return morton.x | (morton.y << 1) | (morton.z << 2);
}

uint GetChildPtr(Node node) {
  return node.childPtr * 8;
}

bool IsNodeEmpty(Node node) {
  return !node.count;
}

bool IsNodeLeaf(Node node) {
  return node.leaf;
}

bool IsNodeNonEmpty(Node node) {
  return node.count && !node.leaf;
}

bool IsNodeFull(Node node) {
  return node.count == 8 && !node.leaf;
}

void TraverseOctree(uint nodeIndex, uint3 nodePosition, uint3 position, uint level, out uint result) 
{
  if (level == MAX_LEVELS) 
  {
    result = nodeIndex;
    return;
  }

  Node node = tree[nodeIndex];

  if (IsNodeEmpty(node))
  {
    result = 0;
    return;
  }

  if (IsNodeLeaf(node)) 
  {
    uint childPtr = GetChildPtr(node);
    uint childIndex = childPtr + EncodeMorton(position - nodePosition * 2u);
    result = childIndex;
    return;
  }

  uint3 childPosition = nodePosition * 2u;
  uint3 childOffset = (position >= childPosition) * 1u;
  uint childIndex = nodeIndex + childOffset.x + childOffset.y * 2u + childOffset.z * 4u;
  TraverseOctree(childIndex, childPosition, position, level + 1u, result);
}

bool IsPositionInsideOctree(uint3 position) 
{
  return all(position < (1u << MAX_LEVELS));
}

void ReadNode(uint nodeIndex, out Node result)
{
  result = tree[nodeIndex];
}

void WriteNode(uint nodeIndex, Node value) 
{
  InterlockedExchange(tree[nodeIndex], value);
}

void InsertNode(uint nodeIndex, Node node) 
{
  uint prevValue = tree[nodeIndex].x;
  uint newValue = (prevValue & 0x80000000) | (node.childPtr << 2) | (node.count << 1) | node.leaf;
  InterlockedExchange(tree[nodeIndex].x, newValue);
}

void CreateLeaf(uint nodeIndex, uint3 position, uint value) {
  Node node = { .childPtr = value, .leaf = 1, .count = 1 };
  WriteNode(nodeIndex, node);
}

void SubdivideNode(uint nodeIndex, uint3 position) 
{
  uint childPtr = InterlockedAdd(treeCounter, 8u) / 8u;
  uint childIndex = GetChildPtr(tree[nodeIndex]) + EncodeMorton(position - (position / 2u) * 2u);

  Node child = { .childPtr = childPtr, .leaf = 1, .count = 0 };
  InsertNode(nodeIndex, child);

  for (uint i = 0; i < 8u; i++) {
    uint3 childPosition = position * 2u + uint3(i & 1u, (i >> 1) & 1u, i >> 2);
    uint childNodeIndex;
    TraverseOctree(0u, uint3(0u, 0u, 0u), childPosition, 0u, childNodeIndex);

    if (IsNodeEmpty(tree[childNodeIndex])) {
      CreateLeaf(childNodeIndex, childPosition, 0u);
    }
  }
}

void IncrementNodeCount(uint nodeIndex) {
  Node node = tree[nodeIndex];
  if (node.count < 8u) {
    InterlockedIncrement(tree[nodeIndex].count);
  }
}

void DecrementNodeCount(uint nodeIndex) {
  InterlockedDecrement(tree[nodeIndex].count);
}

void MergeNode(uint nodeIndex) 
{
  Node node = tree[nodeIndex];

  if (IsNodeLeaf(node)) 
  {
    DecrementNodeCount(nodeIndex);
    return;
  }

  uint8_t childCount = 0;
  uint firstChildIndex = GetChildPtr(node);
  Node firstChild = tree[firstChildIndex];

  for (uint i = 0; i < 8u; i++) 
  {
    Node child = tree[firstChildIndex + i];
    if (!IsNodeEmpty(child)) {
      childCount++;
    }
  }

  if (childCount == 1u && IsNodeNonEmpty(firstChild)) 
  {
    for (uint i = 0; i < 8u; i++) {
      Node child = tree[firstChildIndex + i];
      if (!IsNodeEmpty(child)) {
        uint childIndex = firstChildIndex + i;
        uint grandchildIndex = GetChildPtr(childIndex) + GetChildPtr(child);
        InsertNode(nodeIndex, tree[grandchildIndex]);
      }
    }

    WriteNode(firstChildIndex, Node());
  }
  else if (childCount == 0u) 
  {
    WriteNode(nodeIndex, Node());
  }
}

void UpdateTree(uint3 position, bool value) 
{
  uint nodeIndex;
  TraverseOctree(0u, uint3(0u, 0u, 0u), position, 0u, nodeIndex);

  if (value) {
    IncrementNodeCount(nodeIndex);

    if (IsNodeFull(tree[nodeIndex])) {
      SubdivideNode(nodeIndex, position / 2u);
    }
  } else {
    DecrementNodeCount(nodeIndex);

    if (IsNodeNonEmpty(tree[nodeIndex])) {
      MergeNode(nodeIndex);
    }
  }
}

