

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    float3 voxelId;
    int arrayIndex;
    bool init = false;
};

cbuffer cbSettings : register(b0)
{
    float isoLevel;
    int textureSize;
    float planetSize;
    int pointsPerAxis;
    float3 chunkCoord;
};

Texture3D<float> DensityTexture : register(t0);
StructuredBuffer<int> TriangleTable : register(t1);
RWStructuredBuffer<Vertex> vertices : register(u0);

float3 coordToWorld(int3 coord)
{
    return (coord / (textureSize - 1.0) - 0.5f) * planetSize;
}

int indexFromCoord(int3 coord)
{
    //coord = coord - int3(chunkCoord);
    return coord.z * pointsPerAxis * pointsPerAxis + coord.y * pointsPerAxis + coord.x;
}

float sampleDensity(int3 coord)
{
    coord = max(0, min(coord, textureSize));
    return DensityTexture.Load(float4(coord, 0));
}

float3 calculateNormal(int3 coord)
{
    int3 offsetX = int3(1, 0, 0);
    int3 offsetY = int3(0, 1, 0);
    int3 offsetZ = int3(0, 0, 1);

    float dx = sampleDensity(coord + offsetX) - sampleDensity(coord - offsetX);
    float dy = sampleDensity(coord + offsetY) - sampleDensity(coord - offsetY);
    float dz = sampleDensity(coord + offsetZ) - sampleDensity(coord - offsetZ);

    return normalize(float3(dx, dy, dz));
}

// Calculate the position of the vertex
// The position lies somewhere along the edge defined by the two corner points.
// Where exactly along the edge is determined by the values of each corner point.
Vertex createVertex(int3 coordA, int3 coordB)
{
	
    //float3 posA = coordToWorld(coordA);
    //float3 posB = coordToWorld(coordB);
    float densityA = sampleDensity(coordA);
    float densityB = sampleDensity(coordB);

	// Interpolate between the two corner points based on the density
    float t = (isoLevel - densityA) / (densityB - densityA);
    float3 position = coordA + t * (coordB - coordA);

	// Normal:
    float3 normalA = calculateNormal(coordA);
    float3 normalB = calculateNormal(coordB);
    float3 normal = normalize(normalA + t * (normalB - normalA));

	// ID
    int indexA = indexFromCoord(coordA);
    int indexB = indexFromCoord(coordB);

	// Create vertex
    Vertex vertex;
    vertex.tangent = float3(1, 0, 0);
    vertex.position = position;
    vertex.normal = normal;
    vertex.voxelId = int3(min(indexA, indexB), max(indexA, indexB), 0);
    vertex.init = true;
    
    return vertex;
}

[numthreads(8, 8, 8)]
void GenerateChunk(int3 id : SV_DispatchThreadID)
{
    if (id.x >= pointsPerAxis - 1 || id.y >= pointsPerAxis - 1 || id.z >= pointsPerAxis - 1)
    {
        return;
    }

    int3 coord = id; // + int3(chunkCoord);

    
    
}


[numthreads(4, 4, 4)]
void GenerateQuad(int3 id : SV_DispatchThread)
{
}