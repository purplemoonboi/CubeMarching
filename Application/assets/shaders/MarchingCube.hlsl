#include "MarchingCubeData.hlsli"
#include "PerlinNoise.hlsli"

struct Vertex
{
	float3 position;
    float3 normal;
    float3 tangent;
    float2 id;
};

struct Triangle 
{
	Vertex vertexC;
	Vertex vertexB;
	Vertex vertexA;
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
    int UseGradient;
    int UseTangent;
    float Alpha;
};

Texture3D<float> DensityTexture : register(t0);
StructuredBuffer<int> TriangleTable : register(t1);
RWStructuredBuffer<Triangle> triangles : register(u0);

#define MAX_ITERATIONS 10

float SampleDensity(int3 coord)
{
    float n = 0.0f;
    
    if(UseTexture == 1)
    {
        n = DensityTexture.Load(float4(coord, 0));
    }
    else
    {
        float f = 0.01f;
        float g = 3.0f;
        for (int i = 0; i < 3; i++)
        {
            n += snoise(coord * f);
            f *= g;
        }
    }
    
    return n;
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

void GradientTransformation(float3 c0, float3 n0, float3 c1, float3 n1, inout Vertex v)
{
    //...p = p + a * (n * p)

    float3 dc0 = (float3) 0;
    float3 dc1 = (float3) 0;

    dc0 = c0 + n0 * Alpha;
    dc1 = c1 + n1 * Alpha;
    
    float f0 = SampleDensity(dc0);
    float f1 = SampleDensity(dc1);
    
    float dn0 = CalculateNormal(dc0);
    float dn1 = CalculateNormal(dc1);
    float t = 0.0f;
    
    bool iterate = true;
    
    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        
        dc0 = dc0 + n0 * Alpha;
        dc1 = dc1 + n1 * Alpha;
            
        f0 = SampleDensity(dc0);
        f1 = SampleDensity(dc1);
        
        if (f0 < IsoLevel && f1 < IsoLevel || f0 > IsoLevel && f1 > IsoLevel)
        {
            break;
        }
        else
        {
            dn0 = CalculateNormal(dc0);
            dn1 = CalculateNormal(dc1);
            
            if (f0 < IsoLevel && f1 > IsoLevel)
            {
                t = (IsoLevel - f0) / (f1 - f0);
                v.position = dc0 + t * (dc1 - dc0);
                v.normal = normalize(dn0 + t * (dn1 - dn0));
            }
            if (f0 > IsoLevel && f1 < IsoLevel)
            {
                t = (IsoLevel - f1) / (f0 - f1);
                v.position = dc1 + t * (dc0 - dc1);
                v.normal = normalize(dn1 + t * (dn0 - dn1));
            }
        }
    }
}

float3 EdgeTransform(float3 v, float3 vn, float3 w)
{

    float4x4 I =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
    };
    
    
    float4x4 V =
    {
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { vn.x, vn.y, vn.z, 1.0f },
    };
    
    float4x4 VT = transpose(V);
    float4x4 VVT = mul(V, VT);
    float4x4 T = I - VVT;
    float4 vw = float4(v.xyz - w.xyz, 1.0f);
    float4 dv = mul(T, dv);
    
    return dv.xyz;
}

void TangentialTransformation(int3 vc, int3 wc, inout Vertex v)
{
    
    float3 vp = (float3) 0;

    float3 dnv = CalculateNormal(vc);
    float3 dv = EdgeTransform((float3) vc, dnv, (float3) wc);
    
    float3 dnw = CalculateNormal(wc);
    float3 dw = EdgeTransform((float3) wc, dnw, (float3) vc);
        
    dv = vc + Alpha * dv;
    dw = wc + Alpha * dw;
    
    float f0 = SampleDensity(dv);
    float f1 = SampleDensity(dw);
    
    if (f0 < IsoLevel && f1 < IsoLevel || f0 > IsoLevel && f1 > IsoLevel)
    {
        return;
    }
    
    float t = 0.0f;
    
    dnv = CalculateNormal(vc);
    dnw = CalculateNormal(wc);
    
    if (f0 < IsoLevel && f1 > IsoLevel)
    {
        t = (IsoLevel - f0) / (f1 - f0);
        v.position = dv + t * (dw - dv);
        v.normal = normalize(dnv + t * (dnw - dnv));
    }
    if (f0 > IsoLevel && f1 < IsoLevel)
    {
        t = (IsoLevel - f1) / (f0 - f1);
        v.position = dw + t * (dv - dw);
        v.normal = normalize(dnw + t * (dnv - dnw));
    }

}

Vertex createVertex(int3 c0, int3 c1)
{

    float f0 = SampleDensity(c0);
    float f1 = SampleDensity(c1);
    float t = 0.0f;
    
    float3 normalA = CalculateNormal(c0);
    float3 normalB = CalculateNormal(c1);
    
    Vertex vertex = (Vertex)0;
    
	// Interpolate between the two corner points based on the density
    
    t = (IsoLevel - f0) / (f1 - f0);
    vertex.position = c0 + t * (c1 - c0 );
    vertex.normal = normalize(normalA + t * (normalB - normalA));
    
    if (UseGradient == 1)
    {
        GradientTransformation(c0, normalA, c1, normalB, vertex);
    }
    if (UseTangent == 1)
    {
        //TangentialTransformation(c0, c1, vertex);
    }

    vertex.position = vertex.position / (TextureSize - 1);
    vertex.position *= Resolution;
    vertex.tangent = float3(1, 0, 0);
   

    int indexA = c0.z * Resolution * Resolution + c0.y * Resolution + c0.x;
    int indexB = c1.z * Resolution * Resolution + c1.y * Resolution + c1.x;
    vertex.id = int2(min(indexA, indexB), max(indexA, indexB));

    return vertex;
}

[numthreads(8, 8, 8)]
void GenerateChunk(int3 id : SV_DispatchThreadID)
{
    if (id.x >= TextureSize - 1 || id.y >= TextureSize - 1 || id.z >= TextureSize - 1)
    {
        return;
    }
    
    int3 coord = id +int3(ChunkCoord);

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
        if (SampleDensity(cornerCoords[i]) < IsoLevel)
        {
            cubeConfiguration |= (1 << i);
        }
    }
    
    if(cubeConfiguration == 0 || cubeConfiguration == 255)
        return;
	
    for (i = 0; i < 16; i += 3)
    {
        if (TriangleTable[(cubeConfiguration * 16) + i] == -1)
            break;
        
        int a0 = cornerIndexAFromEdge[TriangleTable[(cubeConfiguration * 16) + i]];
        int a1 = cornerIndexBFromEdge[TriangleTable[(cubeConfiguration * 16) + i]];
        
        int b0 = cornerIndexAFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 1]];
        int b1 = cornerIndexBFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 1]];

        int c0 = cornerIndexAFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 2]];
        int c1 = cornerIndexBFromEdge[TriangleTable[(cubeConfiguration * 16) + i + 2]];

        Triangle tri = (Triangle) 0;
        
        tri.vertexA = createVertex(cornerCoords[a0], cornerCoords[a1]);
        tri.vertexB = createVertex(cornerCoords[b0], cornerCoords[b1]);
        tri.vertexC = createVertex(cornerCoords[c0], cornerCoords[c1]);

        triangles[triangles.IncrementCounter()] = tri;
    }
}
