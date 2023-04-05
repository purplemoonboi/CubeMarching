/* Simple QEF lib */
#include "QEF.hlsli"

struct Vertex
{
    float3 position;
    float3 normal;
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
    float IsoLevel;
    int TextureSize;
    int UseBinarySearch;
    int NumPointsPerAxis;
    float3 ChunkCoord;
    int Resolution;
    int UseTexture;
};

Texture3D<float> DensityTexture : register(t0);
RWStructuredBuffer<Vertex> Vertices : register(u0);
RWStructuredBuffer<Triangle> TriangleBuffer : register(u1);

RWStructuredBuffer<int> VoxelMaterialBuffer : register(u2);

float3 mod289(float3 x)
{
    return x - floor(x / 289.0) * 289.0;
}

float4 mod289(float4 x)
{
    return x - floor(x / 289.0) * 289.0;
}

float4 permute(float4 x)
{
    return mod289((x * 34.0 + 1.0) * x);
}

float4 taylorInvSqrt(float4 r)
{
    return 1.79284291400159 - r * 0.85373472095314;
}

float snoise(float3 v)
{
    const float2 C = float2(1.0 / 6.0, 1.0 / 3.0);

    // First corner
    float3 i = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

    // x1 = x0 - i1  + 1.0 * C.xxx;
    // x2 = x0 - i2  + 2.0 * C.xxx;
    // x3 = x0 - 1.0 + 3.0 * C.xxx;
    float3 x1 = x0 - i1 + C.xxx;
    float3 x2 = x0 - i2 + C.yyy;
    float3 x3 = x0 - 0.5;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    float4 p =
      permute(permute(permute(i.z + float4(0.0, i1.z, i2.z, 1.0))
                            + i.y + float4(0.0, i1.y, i2.y, 1.0))
                            + i.x + float4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float4 j = p - 49.0 * floor(p / 49.0); // mod(p,7*7)

    float4 x_ = floor(j / 7.0);
    float4 y_ = floor(j - 7.0 * x_); // mod(j,N)

    float4 x = (x_ * 2.0 + 0.5) / 7.0 - 1.0;
    float4 y = (y_ * 2.0 + 0.5) / 7.0 - 1.0;

    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

    //float4 s0 = float4(lessThan(b0, 0.0)) * 2.0 - 1.0;
    //float4 s1 = float4(lessThan(b1, 0.0)) * 2.0 - 1.0;
    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, 0.0);

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 g0 = float3(a0.xy, h.x);
    float3 g1 = float3(a0.zw, h.y);
    float3 g2 = float3(a1.xy, h.z);
    float3 g3 = float3(a1.zw, h.w);

    // Normalise gradients
    float4 norm = taylorInvSqrt(float4(dot(g0, g0), dot(g1, g1), dot(g2, g2), dot(g3, g3)));
    g0 *= norm.x;
    g1 *= norm.y;
    g2 *= norm.z;
    g3 *= norm.w;

    // Mix final noise value
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    m = m * m;

    float4 px = float4(dot(x0, g0), dot(x1, g1), dot(x2, g2), dot(x3, g3));
    return 42.0 * dot(m, px);
}

float SampleDensity(int3 coord)
{
    //coord = max(0, min(coord, TextureSize));
    
    if(UseTexture == 0)
    {
        return snoise(coord);
    }
    
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

float3 Interpolate(float3 c0, float3 c1, float f)
{
    return c0.x * (1 - f) + c1.x * f, c0.y * (1 - f) + c1.y * f, c0.z * (1 - f) + c1.z * f;
}

float3 MinimiseError(float3 c0, float3 c1, float f, float s,  int i)
{
    float3 outP = (float3) 0;
    
    if(i == 0)
    {
        outP = Interpolate(c0, c1, f);
    }
    else
    {
        if (snoise(Interpolate(c0, c1, f)) < 0)
        {
            outP = MinimiseError(c0, c1, f + s, s * 0.5f, i - 1);
        }
        else
        {
            outP = MinimiseError(c0, c1, f - s, s * 0.5f, i - 1);
        }
    }
   
    
    return outP;
}

float3 ZeroCrossing(float3 c0, float3 c1)
{
    float3 p =(float3)0;

    float f = 0.5f;
    float s = 0.25f;
    for (int i = 0; i < 10; i++)
    {
        p = MinimiseError(c0, c1, f, s, i);
    }
    
    return p;
}

float Remap(float x, float clx, float cmx, float nlx, float nmx)
{
    return nlx + (x - clx) * (nmx - nlx) / (cmx - clx);
}


[numthreads(8, 8, 8)]
void GenerateVertices(int3 id : SV_DispatchThreadID, int3 gtid : SV_GroupThreadID)
{
    uint index = ((id.z * Resolution ) * Resolution ) + (id.y * Resolution ) + id.x;
    
    
    if (id.x >= Resolution -1  || id.y >= Resolution  - 1 || id.z >= Resolution -1 )
    {
        Vertex vertex = (Vertex) 0;
        vertex.position = float3(100, 0, 0);
        vertex.normal = id;
        vertex.configuration = -1;
        Vertices[index] = vertex;
        return;
    }

    
    int3 coord = id;// + int3(ChunkCoord);

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
        if (DensityTexture[cornerCoords[i]] < IsoLevel)
        {
            cubeConfiguration |= (1 << i);
        }
    }
    
    VoxelMaterialBuffer[index] = cubeConfiguration;
    
    /* voxel is outwith iso threshold */
    if (cubeConfiguration == 0 || cubeConfiguration == 255)
    {
        Vertex vertex = (Vertex) 0;
        vertex.position = float3(0,0,0);
        vertex.normal = id;
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
    float3 p = (float3) 0;
    
    for (i = 0; i < 12 && edgeCount < MAX_EDGE; i++)
    {
        /* fetch indices for the corners of the voxel */
        int c1 = cornerIndexAFromEdge[i];
        int c2 = cornerIndexBFromEdge[i];
        
        /* check the bit at this corner */
        //int m1 = (cubeConfiguration >> c1) & 0xb1;
        //int m2 = (cubeConfiguration >> c2) & 0xb1;
        
        float3 p1 = cornerCoords[c1];
        float3 p2 = cornerCoords[c2];
        
        float g1 = DensityTexture[p1];
        float g2 = DensityTexture[p2];
        
        int m1 = (g1 > 0) ? 1 : 0;
        int m2 = (g2 > 0) ? 1 : 0;
        
        
        /* if there is a sign change */
        if (!((m1 == 0 && m2 == 0) || (m1 == 1 && m2 == 1)))
        {

            float t = (IsoLevel - DensityTexture[p1]) / (DensityTexture[p2] - DensityTexture[p1]);

            float3 p = p1 + t * (p2 - p1);
            float3 n = CalculateNormal(p);
            
            p.x = Remap(p.x, 0.0f, 128.0f, 0.0f, 1.0f);
            p.y = Remap(p.y, 0.0f, 128.0f, 0.0f, 1.0f);
            p.z = Remap(p.z, 0.0f, 128.0f, 0.0f, 1.0f);
            
            //p2.x = Remap(p2.x, 0.0f, 128.0f, 0.0f, 1.0f);
            //p2.y = Remap(p2.y, 0.0f, 128.0f, 0.0f, 1.0f);
            //p2.z = Remap(p2.z, 0.0f, 128.0f, 0.0f, 1.0f);
            
           
            
            
            qef_add(n, p, ATA, Atb, pointaccum, btb);
            averageNormal += n;
            edgeCount++;

        }
    }

    
    averageNormal = normalize(averageNormal / edgeCount);
    
    
   
    float3 com = float3(pointaccum.x, pointaccum.y, pointaccum.z) / pointaccum.w;
    float3 solvedPosition = com.xyz;
    
    float error = qef_solve(ATA, Atb, com.xyz, solvedPosition);
    
    float3 minimum = cornerCoords[0];
    float3 maximum = cornerCoords[0] + float3(1, 1, 1);

    /* sometimes the position generated spawns the vertex outside the voxel */
    /* if this happens place the vertex at the centre of mass */
    
    if(solvedPosition.x < 0)
        solvedPosition.x *= -1.0f;

    if (solvedPosition.y < 0)
        solvedPosition.y *= -1.0f;
    
    if (solvedPosition.z < 0)
        solvedPosition.z *= -1.0f;
    
    solvedPosition.x = Remap(solvedPosition.x, 0.0f, 1.0f, 0.0f, 32.0f);
    solvedPosition.y = Remap(solvedPosition.y, 0.0f, 1.0f, 0.0f, 32.0f);
    solvedPosition.z = Remap(solvedPosition.z, 0.0f, 1.0f, 0.0f, 32.0f);
    
    
    if (solvedPosition.x < minimum.x || solvedPosition.y < minimum.y || solvedPosition.z < minimum.z ||
    solvedPosition.x > maximum.x || solvedPosition.y > maximum.y || solvedPosition.z > maximum.z)
    {
        solvedPosition.xyz = com.xyz;
    }
    
    
            

    
    Vertex vertex = (Vertex) 0;
    
    //solvedPosition = solvedPosition / (TextureSize - 1);
    solvedPosition *= 32;
    
    
    
    vertex.position = solvedPosition.xyz;
    vertex.normal = averageNormal;
    vertex.configuration = 1;
    
    Vertices[index] = vertex;
    
}


[numthreads(1,1,1)]
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
    
    int3 right      = id + int3(1, 0, 0);
    int3 up         = id + int3(0, 1, 0);
    int3 forward    = id + int3(0, 0, 1);
    
   
    /* we only want to check three times per voxel starting from the corner */
    int3 coord = id ;//+ int3(ChunkCoord);

   
    Triangle tri = (Triangle) 0;
    
    /* 'this' voxel */
    uint pxyz = ((id.z * Resolution ) * Resolution ) + (id.y * Resolution ) + id.x;
    
    uint left_and_below_z = ((id.z * Resolution ) * Resolution ) + ((id.y - 1) *Resolution ) + (id.x - 1);
    
    uint left_z = ((id.z * Resolution ) * Resolution ) + (id.y *Resolution ) + (id.x - 1);
    
    /* along the y-axis */
    uint left_of_y = ((id.z * Resolution ) * Resolution ) + (id.y * Resolution ) + (id.x - 1);
    
    uint left_and_behind_Y = (((id.z - 1) * Resolution ) * Resolution ) + (id.y * Resolution ) + (id.x - 1);
    
    uint behind_y = (((id.z - 1) * Resolution ) * Resolution ) + (id.y * Resolution ) + id.x;
    
    /* along the x-axis */
    uint below_pxyz = ((id.z * Resolution ) * Resolution ) + ((id.y - 1) * Resolution ) + id.x;

    uint right_of_and_below_x = (((id.z - 1) * Resolution ) * Resolution ) + ((id.y - 1) * Resolution ) + id.x;
    
    uint right_of_x = (((id.z - 1) * Resolution ) * Resolution ) + (id.y * Resolution ) + id.x;
   
    
    
    uint right_of_and_behind_y = (((id.z - 1) * Resolution ) * Resolution ) + (id.y * Resolution ) + id.x;
    
    
    /*
    *
    *   If the sign change is flipped i.e the edge extrudes from air into 
    *   solid from our perspective. Then build the triangles like so...
    *
    */
    
    float g0 = snoise(coord);
    float gf = snoise(forward);
    float gr = snoise(right);
    float gu = snoise(up);
    /* check the z-axis */
    if (DensityTexture[coord] < 0 && DensityTexture[forward] > 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_z].configuration == 1 &&
            Vertices[left_and_below_z].configuration == 1)
        {
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_z];
            tri.vertexC = Vertices[left_and_below_z];
            
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_and_below_z].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_below_z];
            tri.vertexC = Vertices[below_pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
      
    if (DensityTexture[coord] > 0 && DensityTexture[forward] < 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[left_and_below_z].configuration == 1 &&
            Vertices[left_z].configuration == 1 &&
            Vertices[pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[left_and_below_z];
            tri.vertexB = Vertices[left_z];
            tri.vertexC = Vertices[pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[left_and_below_z].configuration == 1 &&
            Vertices[pxyz].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[left_and_below_z];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[below_pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
    
    /* check the x-axes */
    if (DensityTexture[coord] < 0 && DensityTexture[right] > 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[right_of_x].configuration == 1 &&
            Vertices[pxyz].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1)
        {
        
            tri.vertexA = Vertices[right_of_x];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[below_pxyz];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        
        if (Vertices[right_of_x].configuration == 1 &&
            Vertices[below_pxyz].configuration == 1 &&
            Vertices[right_of_and_below_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[right_of_x];
            tri.vertexB = Vertices[below_pxyz];
            tri.vertexC = Vertices[right_of_and_below_x];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
    if (DensityTexture[coord] > 0 && DensityTexture[right] < 0)
    {
         /* ...sweep around the pxyz axes and append the vertices */
        if (Vertices[below_pxyz].configuration == 1 &&
            Vertices[pxyz].configuration == 1 &&
            Vertices[right_of_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[below_pxyz];
            tri.vertexB = Vertices[pxyz];
            tri.vertexC = Vertices[right_of_x];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
        
        if (Vertices[below_pxyz].configuration == 1 &&
            Vertices[right_of_x].configuration == 1 &&
            Vertices[right_of_and_below_x].configuration == 1)
        {
        
            tri.vertexA = Vertices[below_pxyz];
            tri.vertexB = Vertices[right_of_x];
            tri.vertexC = Vertices[right_of_and_below_x];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }

    /* check the y-axis */
    
    if (DensityTexture[coord] < 0 && DensityTexture[up] > 0)
    {
        
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[right_of_x].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1)
        {
        
        
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[right_of_x];
            tri.vertexC = Vertices[left_and_behind_Y];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
       
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1 &&
            Vertices[left_z].configuration == 1)
        {
       
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_behind_Y];
            tri.vertexC = Vertices[left_z];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
    
    if (DensityTexture[coord] > 0 && DensityTexture[up] < 0)
    {
        /* ...sweep around the pxyz axes and append the vertices */
        
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_of_y].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1)
        {
            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_of_y];
            tri.vertexC = Vertices[left_and_behind_Y];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
       
        if (Vertices[pxyz].configuration == 1 &&
            Vertices[left_and_behind_Y].configuration == 1 &&
            Vertices[behind_y].configuration == 1)
        {

            tri.vertexA = Vertices[pxyz];
            tri.vertexB = Vertices[left_and_behind_Y];
            tri.vertexC = Vertices[behind_y];
        
            TriangleBuffer[TriangleBuffer.IncrementCounter()] = tri;
        }
    }
    
 
    
}