/* Simple QEF lib */
#include "QEF.hlsli"

struct Vertex
{
    float3 position;
    float3 normal;
    float3 tangent;
    int configuration;
};

struct Triangle
{
    Vertex vertexA;
    Vertex vertexB;
    Vertex vertexC;
};

cbuffer cbSettings : register(b0)
{
    float IsoValue;
    int TextureSize;
    int UseBinarySearch; //Toggle for alternating between QEF and Binary Search
    int NumOfPointsPerAxis;
    float3 ChunkCoord;
    int Resolution;
    int UseTexture;
    int UseGradient;
    int UseTangent;
    float Alpha;
    int SurfaceNets;
};

Texture3D<float> DensityTexture : register(t0);
RWStructuredBuffer<Vertex> Vertices : register(u0);
RWStructuredBuffer<Triangle> TriangleBuffer : register(u1);

RWStructuredBuffer<int> VoxelMaterialBuffer : register(u2);



float SampleDensity(int3 coord)
{
    //coord = max(0, min(coord, TextureSize));
    return DensityTexture.Load(float4(coord, 0));
    
}

float3 CalculateNormal(int3 coord)
{
    int3 offsetX = int3(1, 0, 0);
    int3 offsetY = int3(0, 1, 0);
    int3 offsetZ = int3(0, 0, 1);

    float dx = SampleDensity(coord + offsetX) - SampleDensity(coord - offsetX);
    float dy = SampleDensity(coord + offsetY) - SampleDensity(coord - offsetY);
    float dz = SampleDensity(coord + offsetZ) - SampleDensity(coord - offsetZ);

    return normalize(float3(dx, dy, dz));
}

static int2 EdgeMap[12] =
{
    int2(0, 4), int2(1, 5), int2(2, 6), int2(3, 7),
	int2(0, 2), int2(1, 3), int2(4, 6), int2(5, 7),
	int2(0, 1), int2(2, 3), int2(4, 5), int2(6, 7)
};



static const uint cornerIndexAFromEdge[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
static const uint cornerIndexBFromEdge[12] = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };



[numthreads(8, 8, 8)]
void GenerateVertices(int3 id : SV_DispatchThreadID, int3 gtid : SV_GroupThreadID)
{
    uint index = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    
    
    if (id.x >= TextureSize - 1 || id.y >= TextureSize - 1 || id.z >= TextureSize - 1)
    {
        Vertex vertex = (Vertex) 0;
        vertex.position = float3(100, 0, 0);
        vertex.normal = id;
        vertex.tangent = id;
        vertex.configuration = -1;
        Vertices[index] = vertex;
        return;
    }

    
    int3 coord = id; // + int3(ChunkCoord);

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
        if (DensityTexture[cornerCoords[i]] < IsoValue)
        {
            cubeConfiguration |= (1 << i);
        }
    }
    
    VoxelMaterialBuffer[index] = cubeConfiguration;
    
    /* voxel is outwith iso threshold */
    if (cubeConfiguration == 0 || cubeConfiguration == 255)
    {
        Vertex vertex = (Vertex) 0;
        vertex.position = float3(0, 0, 0);
        vertex.normal = id;
        vertex.tangent = id;
        
        vertex.configuration = -1;
        Vertices[index] = vertex;
        
        return;
    }
     
    int MAX_EDGE = 6;
    float ATA[6] = { 0, 0, 0, 0, 0, 0 };
    float4 pointaccum = (float4) 0;
    float3 Atb = (float3) 0;
    float3 averageNormal = (float3) 0;
    float btb = (float) 0;
    float edgeCount = 0;
    
    /* for surface nets algo */
    float3 avgPosition = (float3) 0;
    
    for (i = 0; i < 12 && edgeCount < MAX_EDGE; i++)
    {
        /* fetch indices for the corners of the voxel */
        int c1 = cornerIndexAFromEdge[i];
        int c2 = cornerIndexBFromEdge[i];
        
        /* check the bit at this corner */
        float3 p1 = cornerCoords[c1];
        float3 p2 = cornerCoords[c2];
        
        float s0 = DensityTexture[p1];
        float s1 = DensityTexture[p2];
        

        
        int m1 = (s0 > 0) ? 1 : 0;
        int m2 = (s1 > 0) ? 1 : 0;
        
        /* if there is a sign change */
        if (!((m1 == 0 && m2 == 0) || (m1 == 1 && m2 == 1)))
        {
            /* ...calculate the intersecting vertex along the edge */
            p1 = cornerCoords[c1];
            p2 = cornerCoords[c2];
            
            float t = (IsoValue - s0) / (s1 - s0);
            float3 p = p1 + t * (p2 - p1);
            
            float3 n = CalculateNormal(p);
            avgPosition += p;
            
            QEFAdd(n, p, ATA, Atb, pointaccum, btb);
            averageNormal += n;
            edgeCount++;
            
        }
    }

    avgPosition /= edgeCount;
    
    averageNormal = normalize(averageNormal / edgeCount);
    float3 com = float3(pointaccum.x, pointaccum.y, pointaccum.z) / pointaccum.w;
    float3 solvedPosition = (float3) 0;
    
    float error = SolveQEF(ATA, Atb, com, solvedPosition);
    float3 minimum = cornerCoords[0];
    float3 maximum = cornerCoords[0] + float3(1, 1, 1);

    /* sometimes the position generated spawns the vertex outside the voxel */
    /* if this happens place the vertex at the centre of mass */
    
    if (solvedPosition.x < minimum.x || solvedPosition.y < minimum.y || solvedPosition.z < minimum.z ||
        solvedPosition.x > maximum.x || solvedPosition.y > maximum.y || solvedPosition.z > maximum.z)
    {
        solvedPosition.xyz = com.xyz;
    }
    
    Vertex vertex = (Vertex) 0;
    
    solvedPosition = solvedPosition / (TextureSize - 1);
    solvedPosition *= Resolution;
    
    vertex.position = avgPosition;
    vertex.normal = averageNormal;
    vertex.tangent = id;
    
    vertex.configuration = 1;
    
    Vertices[index] = vertex;
    
}

bool QueryDistance(Vertex a, Vertex b, uint r)
{
    bool valid = false;
    
    //...1.0f - The min distance between voxel midpoints.
    float err = 1.0f + 1.0f;
    if (length(b.position - a.position) <= err || length(a.position - b.position) <= err)
    {
        valid = true;
    }
    
    return valid;
}

[numthreads(1, 1, 1)]
void GenerateTriangle(uint3 id : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    
   /*   @brief
    *
    *   Each thread will work on an edge in parallel.
    *
    *   We check each direction again for a sign change. { right -> up -> forward }
    * 
    *   If there is a sign change along that edge...
    *   ...then we know the voxels parallel to the edge must contain vertices.
    *
    *   Using the voxel coord as the index into the vertex buffer... 
    *
    *   ...we append the vertices in anti-clockwise order to build the 
    *   triangle.
    *
    *   @note This is the naive approach and can be improved with Sparse octrees.
    */
    
    int3 right = id + int3(1, 0, 0);
    int3 up = id + int3(0, 1, 0);
    int3 forward = id + int3(0, 0, 1);
    
   
    /* we only want to check three times per voxel starting from the corner */


    int3 coord = id; //+ int3(ChunkCoord);

   
    Triangle tri = (Triangle) 0;
    

    
    /* 'this' voxel */
    uint pxyz = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    uint left_and_below_z = ((id.z * Resolution) * Resolution) + ((id.y - 1) * Resolution) + (id.x - 1);
    
    uint left_z = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    /* along the y-axis */
    uint left_of_y = ((id.z * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    uint left_and_behind_Y = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + (id.x - 1);
    
    uint behind_y = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    /* along the x-axis */
    uint below_pxyz = ((id.z * Resolution) * Resolution) + ((id.y - 1) * Resolution) + id.x;

    uint right_of_and_below_x = (((id.z - 1) * Resolution) * Resolution) + ((id.y - 1) * Resolution) + id.x;
    
    uint right_of_x = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + id.x;
   
    uint right_of_and_behind_y = (((id.z - 1) * Resolution) * Resolution) + (id.y * Resolution) + id.x;
    
    
    /*
    *
    *   If the sign change is flipped i.e the edge extrudes from air into 
    *   solid from our perspective. Then build the triangles like so...
    *
    */
    
    /* check the z-axis */
    if (DensityTexture[coord] > 0 && DensityTexture[forward] < 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_z].configuration == 1 &&
            Vertices[left_and_below_z].configuration == 1)
        {
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_z];
            tri.vertexC = Vertices[left_and_below_z];
            
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) && 
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) && 
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
        
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_and_below_z].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_below_z];
            tri.vertexC = Vertices[below_pxyz];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
    }
    
      
    if (DensityTexture[coord] < 0 && DensityTexture[forward] > 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[left_and_below_z].configuration == 1 &&
            Vertices[left_z].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[left_and_below_z];
            tri.vertexB = Vertices[left_z];
            tri.vertexC = Vertices[pxyz];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
        
        if (Vertices[left_and_below_z].configuration == 1 &&
            Vertices[pxyz].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[left_and_below_z];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[below_pxyz];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
    }
    
    
    /* check the x-axes */
    if (DensityTexture[coord] > 0 && DensityTexture[right] < 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[right_of_x].configuration == 1 &&
            Vertices[pxyz].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[right_of_x];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[below_pxyz];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
        
        
        if (Vertices[right_of_x].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1 &&
            Vertices[right_of_and_below_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[right_of_x];
            tri.vertexB = Vertices[below_pxyz];
            tri.vertexC = Vertices[right_of_and_below_x];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
    }
    
    if (DensityTexture[coord] < 0 && DensityTexture[right] > 0)
    {
         /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[below_pxyz].configuration == 1 &&
            Vertices[pxyz].configuration == 1 &&
            Vertices[right_of_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[below_pxyz];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[right_of_x];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
        
        if (Vertices[below_pxyz].configuration == 1 &&
            Vertices[right_of_x].configuration == 1 &&
            Vertices[right_of_and_below_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[below_pxyz];
            tri.vertexB = Vertices[right_of_x];
            tri.vertexC = Vertices[right_of_and_below_x];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
    }

    /* check the y-axis */
    
    if (DensityTexture[coord] > 0 && DensityTexture[up] < 0)
    {
        
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[right_of_x].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1)
        {
        
        
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[right_of_x];
            tri.vertexC = Vertices[left_and_behind_Y];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
       
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1 &&
            Vertices[left_z].configuration == 1)
        {
       
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_behind_Y];
            tri.vertexC = Vertices[left_z];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
    }
    
    
    if (DensityTexture[coord] < 0 && DensityTexture[up] > 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_of_y].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1)
        {
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_of_y];
            tri.vertexC = Vertices[left_and_behind_Y];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
       
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1 &&
            Vertices[behind_y].configuration == 1)
        {

            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_behind_Y];
            tri.vertexC = Vertices[behind_y];
        
            if (QueryDistance(tri.vertexA, tri.vertexB, Resolution) &&
                QueryDistance(tri.vertexB, tri.vertexC, Resolution) &&
                QueryDistance(tri.vertexC, tri.vertexA, Resolution))
            {
                TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
            }
        }
    }
    
 
    
}