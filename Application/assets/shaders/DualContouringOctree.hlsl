#include "SVD.hlsli"
#include "DensityPrimitives.hlsli"

struct Voxel
{
    float3 position;
    float3 normal;
    int numPoints;
};

struct Quad
{
    Voxel VertexA;
    Voxel VertexB;
    Voxel VertexC;
    Voxel VertexD;
};

struct DensityPrimitive
{
    int type;
    int csg;
    float3 position;
    float3 size;
};

cbuffer WorldSettings : register(b0)
{
    float3 ChunkPosition;
    int Resolution;
    int OctreeSize;
	int PrimitiveCount;
}

RWStructuredBuffer<Voxel> Voxels : register(u0);
RWStructuredBuffer<uint> CornerMaterials : register(u1);
RWStructuredBuffer<uint> VoxelMaterials  : register(u2);
RWStructuredBuffer<uint> CornerIndexes   : register(u3);
RWStructuredBuffer<float3> VoxelMins : register(u4);

RWStructuredBuffer<int> CornerCount : register(u5);
RWStructuredBuffer<int> FinalCount : register(u6);

RWStructuredBuffer<DensityPrimitive> Primitives : register(u7);



static float3 VoxelCornerOffsets[8] = { float3(0, 0, 0), float3(0, 0, 1), float3(0, 1, 0), float3(0, 1, 1), float3(1, 0, 0), float3(1, 0, 1), float3(1, 1, 0), float3(1, 1, 1) };

static int2 EdgeMap[12] =
{
    int2(0, 4), int2(1, 5), int2(2, 6), int2(3, 7),
	int2(0, 2), int2(1, 3), int2(4, 6), int2(5, 7),
	int2(0, 1), int2(2, 3), int2(4, 5), int2(6, 7)
};

// Noise Shader Library for Unity - https://github.com/keijiro/NoiseShader
//
// Original work (webgl-noise) Copyright (C) 2011 Ashima Arts.
// Translation and modification was made by Keijiro Takahashi.
//
// This shader is based on the webgl-noise GLSL shader. For further details
// of the original shader, please see the following description from the
// original source code.
//

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//

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

float4 snoise_grad(float3 v)
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

    // Compute noise and gradient at P
    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    float4 m2 = m * m;
    float4 m3 = m2 * m;
    float4 m4 = m2 * m2;
    float3 grad =
        -6.0 * m3.x * x0 * dot(x0, g0) + m4.x * g0 +
        -6.0 * m3.y * x1 * dot(x1, g1) + m4.y * g1 +
        -6.0 * m3.z * x2 * dot(x2, g2) + m4.z * g2 +
        -6.0 * m3.w * x3 * dot(x3, g3) + m4.w * g3;
    float4 px = float4(dot(x0, g0), dot(x1, g1), dot(x2, g2), dot(x3, g3));
    return 42.0 * float4(grad, dot(m4, px));
}

static float FractalNoise(int octaves, float frequency, float lacunarity, float persistence, float3 position)
{
    float SCALE = 1.0f / 128.0f;
    float3 p = position * SCALE;
    float nois = 0.0f;

    float amplitude = 1.0f;
    p *= frequency;

    for (int i = 0; i < octaves; i++)
    {
        nois += snoise_grad(p) * amplitude;
        p *= lacunarity;
        amplitude *= persistence;
    }
	
    return nois;
}

static float FractalNoise(float frequency, float lacunarity, float persistence, float3 position)
{
    float SCALE = 1.0f / 128.0f;
    float3 p = position * SCALE;
    float noise = 0.0f;

    float amplitude = 1.0f;
    p *= frequency;
	
    noise += snoise_grad(p) * amplitude;
    p *= lacunarity;
    amplitude *= persistence;
	
    return noise;
}

static float CalculateNoiseValue(float3 pos, float scale)
{
    return FractalNoise(4, 0.5343f, 2.2324f, 0.68324f, pos * scale);
}

static float CLerp(float a, float b, float t)
{
    return (1 - t) * a + t * b;
}

float SimplexDensityFunction(float3 worldPosition)
{
 //   float worldRadius = 200.0f;
 //   float3 world = worldPosition - float3(0, -worldRadius, 0);
 //   float worldDist = -worldRadius + length(world);

 //   float flatlandNoiseScale = 1.0f;
 //   float flatlandLerpAmount = 0.07f;
 //   float flatlandYPercent = 1.2f;

 //   float rockyNoiseScale = 1.5f;
 //   float rockyLerpAmount = 0.05f;
 //   float rockyYPercent = 0.7f;
	
 //   float maxMountainMixLerpAmount = 0.075f;
 //   float minMountainMixLerpAmount = 1.0f;
	
 //   float mountainBlend = 0.0f;
 //   float rockyBlend = 1.0f;
	
 //   mountainBlend = saturate(abs(FractalNoise(0.5343f, 2.2324f, 0.68324f, world * 0.11f)) * 4.0f);
	
	////float rockiness = abs(FractalNoise(0.5343f, 2.2324f, 0.68324f, world * 0.05f) * 2.0f);
	////rockyBlend = saturate(rockiness);
	
	////if (worldPosition.y < -15.0f)
	//	//mountainBlend = 0;
	
	
 //   float mountain = CalculateNoiseValue(world, 0.07f);

 //   float blob = CalculateNoiseValue(world, flatlandNoiseScale + ((rockyNoiseScale - flatlandNoiseScale) * rockyBlend));
 //   blob = CLerp(blob, (worldDist) * (flatlandYPercent + ((rockyYPercent - flatlandYPercent) * rockyBlend)),
	//			flatlandLerpAmount + ((rockyLerpAmount - flatlandLerpAmount) * rockyBlend));

 //   float result = ((worldDist) / worldRadius) + CLerp(mountain, blob, minMountainMixLerpAmount + ((maxMountainMixLerpAmount - minMountainMixLerpAmount) * mountainBlend));
	
	
    //for (int i = 0; i < PrimitiveCount; i++)
    //{
    //    float primitive = 0;
    //    bool primChosen = Primitives[i].type == 0 || Primitives[i].type == 1 || Primitives[i].type == 2;
		
    //    if (primChosen)
    //    {
    //        if (Primitives[i].type == 0)
    //        {
    //            primitive = Box(worldPosition, Primitives[i].position, Primitives[i].size);
    //        }
    //        else if (Primitives[i].type == 1)
    //        {
    //            primitive = Sphere(worldPosition, Primitives[i].position, Primitives[i].size.x);
    //        }
    //        else if (Primitives[i].type == 2)
    //        {
    //            primitive = Cylinder(worldPosition, Primitives[i].position, Primitives[i].size);
    //        }
		
    //        if (Primitives[i].csg == 0)
    //        {
    //            result = max(-primitive, result);
    //        }
    //        else
    //        {
    //            result = min(primitive, result);
    //        }
    //    }
    //}
    
   /*
    *  for now we're just calculate the simplex noise
    */
    float result = snoise(worldPosition * 0.01f);
	
    return result;
}

static float3 ApproximateZeroCrossingPosition(float3 p0, float3 p1)
{
    float minValue = 100000.0f;
    float t = 0.0f;
    float currentT = 0.0f;
    float steps = 8;
    float increment = 1.0f / steps;
    while (currentT <= 1.0f)
    {
        float3 p = p0 + ((p1 - p0) * currentT);
        float density = abs(SimplexDensityFunction(p));
        if (density < minValue)
        {
            minValue = density;
            t = currentT;
        }

        currentT += increment;
    }

    return p0 + ((p1 - p0) * t);
}

static float3 CalculateSurfaceNormal(float3 p)
{
    float H = 0.001f;
    float dx = SimplexDensityFunction(p + float3(H, 0.0f, 0.0f)) - SimplexDensityFunction(p - float3(H, 0.0f, 0.0f));
    float dy = SimplexDensityFunction(p + float3(0.0f, H, 0.0f)) - SimplexDensityFunction(p - float3(0.0f, H, 0.0f));
    float dz = SimplexDensityFunction(p + float3(0.0f, 0.0f, H)) - SimplexDensityFunction(p - float3(0.0f, 0.0f, H));

    return normalize(float3(dx, dy, dz));
}

[numthreads(9, 8, 8)]
void ComputeMaterials(/*int3 threadID : SV_GroupThreadID, int3 groupID : SV_GroupID,*/ uint3 id : SV_DispatchThreadID)
{
    /*
    *   Offset the worker into our grid
    */
    uint threadIndex = id.x + 9 * (id.y + 8 * id.z);
	
    /*
    *  Calculate the unit size of our bounding volume
    */
    float fResolution = (float) (Resolution + 1);
    float sqFResolution = sqrt(fResolution * fResolution * fResolution);
    
    /*
    *   Adjust the singed resolution value if after casting  
    *   value is rounded down.
    */
    int sqIResolution = (int) sqFResolution;
    if (sqFResolution > (float) sqIResolution)
    {
        sqIResolution = sqIResolution + 1;
    }
	
    /*
    *  Calculate the size of a voxel with respect to the permitted maximum declared size 
    *  and the size of the octree
    */
    int nodeSize = (int) ((uint) HIGHEST_RESOLUTION) / ((uint) OctreeSize);
	
    /*
    *   If our thread is within the bounds of the bounding volume...
    */
    if (threadIndex < (uint) sqIResolution)
    {
        uint uResolution = (uint) (Resolution + 1);
		
        for (int i = 0; i < sqIResolution; i++)
        {
            /*
            *   Calculate the density value at each corner in the Voxels
            *   using a blend between noise and density primitives.
            */
            uint index = (threadIndex * (uint) sqIResolution) + i;
            uint z = round(index / (uResolution * uResolution));
            uint y = round((index - z * uResolution * uResolution) / uResolution);
            uint x = index - uResolution * (y + uResolution * z);
			
            float3 cornerPos = float3((float) x * nodeSize, (float) y * nodeSize, (float) z * nodeSize);
            float density = SimplexDensityFunction(cornerPos + ChunkPosition);

            uint material = density < 0.0f ? 1 : 0;
            CornerMaterials[x + uResolution * (y + uResolution * z)] = material;
        }
    }
}

[numthreads(8, 8, 8)]
void ComputeCorners(/*int3 threadID : SV_GroupThreadID, int3 groupID : SV_GroupID,*/ uint3 id : SV_DispatchThreadID)
{
    
    /*
    *  Offset into our octree with respect to the thread worker
    */
    uint threadIndex = id.x + 8 * (id.y + 8 * id.z);
    
    /*
    *  Calculate the length of our octree
    */
//    float fResolution = (float) Resolution;
    float fResolution = (float) 64U;
    float sqFResolution = sqrt((fResolution * fResolution * fResolution));
    int sqIResolution = (int) sqFResolution;
    
    if (sqFResolution > (float) sqIResolution)
    {
        sqIResolution = sqIResolution + 1;
    }
	
    /*
    *  Calculate the size of a voxel with respect to the permitted maximum declared size 
    *  and the size of the octree
    */
 //   int nodeSize = (int) ((uint) HIGHEST_RESOLUTION) / ((uint) OctreeSize);
    int nodeSize = (int) ((uint) HIGHEST_RESOLUTION) / ((uint) 8U);
	
    if (threadIndex < (uint) sqIResolution)
    {
        uint uResolution = (uint) 64;
		
        for (int i = 0; i < sqIResolution; i++)
        {
            /*
            *    Calculate the position of the node
            */
            uint index = (threadIndex * (uint) sqIResolution) + i;
            uint z = round(index / (uResolution * uResolution));
            uint y = round((index - z * uResolution * uResolution) / uResolution);
            uint x = index - uResolution * (y + uResolution * z);
			
            float3 cornerPos = float3((float) x * nodeSize, (float) y * nodeSize, (float) z * nodeSize);
            //float density = SimplexDensityFunction(cornerPos + ChunkPosition);
            float density = SimplexDensityFunction(cornerPos + float3(0, 0, 0));
            uint material = density < 0.0f ? 1 : 0;
            CornerMaterials[x + uResolution * (y + uResolution * z)] = material;
            
            uint corners = 0;
            
            /*
            *   Sample the eight corners
            */
            for (int j = 0; j < 8; j++)
            {
                uint3 nodePos = uint3(x, y, z);
                uint3 cornerPos = nodePos + uint3(VoxelCornerOffsets[j].x, VoxelCornerOffsets[j].y, VoxelCornerOffsets[j].z);
                uint material = CornerMaterials[cornerPos.x + (uResolution + 1) * (cornerPos.y + (uResolution + 1) * cornerPos.z)];
                corners |= (material << j);
            }
			
            VoxelMaterials[x + uResolution * (y + uResolution * z)] = corners;
			
            if (corners != 0 && corners != 255)
            {
                CornerCount[threadIndex] = CornerCount[threadIndex] + 1;
            }
        }
    }
}

[numthreads(1, 1, 1)]
void AddLength(int3 threadID : SV_GroupThreadID, int3 groupID : SV_GroupID, uint3 id : SV_DispatchThreadID)
{
    float fR = (float) Resolution;
    float sqRTRC = sqrt((fR * fR * fR));
    int sqRTRes = (int) sqRTRC;
    if (sqRTRC > (float) sqRTRes)
    {
        sqRTRes = sqRTRes + 1;
    }
	
    for (int i = 0; i < sqRTRes; i++)
    {
        FinalCount[0] += CornerCount[i];
    }
}

[numthreads(8, 8, 8)]
void ComputePositions(/*int3 threadID : SV_GroupThreadID, int3 groupID : SV_GroupID,*/ uint3 id : SV_DispatchThreadID)
{
    /*
    * offset to the true thread index
    */
    uint threadIndex = (uint) id.x + 8 * ((uint) id.y + 8 * (uint) id.z);
	
    /*
    * calculate the unit resolution
    */
    float fR = (float) Resolution;
    float sqRTRC = sqrt((fR * fR * fR));
    int sqRTRes = (int) sqRTRC;
    if (sqRTRC > (float) sqRTRes)
    {
        sqRTRes = sqRTRes + 1;
    }
	
    /*
    *   If our thread Id is within the octree bounds...
    */
    if (threadIndex < (uint) sqRTRes)
    {
        /*
        *   ...then sum the voxels which exhibit a sign change 
        */
        uint pre = 0;
        for (uint c = 0; c < threadIndex; c++)
        {
            pre += CornerCount[c];
        }
		
        uint uResolution = (uint) Resolution;
		
        uint count = 0;
        for (int i = 0; i < sqRTRes; i++)
        {
            uint index = (threadIndex * (uint) sqRTRes) + i;
            uint z = round(index / (uResolution * uResolution));
            uint y = round((index - z * uResolution * uResolution) / uResolution);
            uint x = index - uResolution * (y + uResolution * z);
			
            uint corners = VoxelMaterials[x + uResolution * (y + uResolution * z)];
			
            if (corners != 0 && corners != 255)
            {
                CornerIndexes[pre + count] = x + uResolution * (y + uResolution * z);
                count++;
            }
        }
    }
}

[numthreads(128, 1, 1)]
void ComputeVoxels(/*int3 threadID : SV_GroupThreadID, int3 groupID : SV_GroupID,*/ uint3 id : SV_DispatchThreadID)
{
    int trueIndex = id.x;
    int count = (int) FinalCount[0];
	
    if (trueIndex < count)
    {
        uint uResolution = (uint) Resolution;
	
        int nodeSize = (int) ((uint) HIGHEST_RESOLUTION) / ((uint) OctreeSize);
	
        uint voxelIndex = CornerIndexes[trueIndex];
        
        /*
        *   Calculate the node's position
        */
        uint z = round(voxelIndex / (uResolution * uResolution));
        uint y = round((voxelIndex - z * uResolution * uResolution) / uResolution);
        uint x = voxelIndex - uResolution * (y + uResolution * z);

        uint corners = VoxelMaterials[(int) voxelIndex];

        float3 nodePos = float3((float) x * nodeSize, (float) y * nodeSize, (float) z * nodeSize) + ChunkPosition;
        VoxelMins[trueIndex] = nodePos;

        int MAX_CROSSINGS = 6;
        int edgeCount = 0;
		
        float4 pointaccum = float4(0, 0, 0, 0);
        mat3x3_tri ATA = { 0, 0, 0, 0, 0, 0 };
        float4 Atb = float4(0, 0, 0, 0);
        float3 averageNormal = float3(0, 0, 0);
        float btb = 0;

        for (int j = 0; j < 12 && edgeCount <= MAX_CROSSINGS; j++)
        {
            int c1 = EdgeMap[j].x;
            int c2 = EdgeMap[j].y;

            int m1 = (corners >> c1) & 1;
            int m2 = (corners >> c2) & 1;
			
            if (!((m1 == 0 && m2 == 0) || (m1 == 1 && m2 == 1)))
            {
                float3 p1 = nodePos + (VoxelCornerOffsets[c1] * nodeSize);
                float3 p2 = nodePos + (VoxelCornerOffsets[c2] * nodeSize);
                float3 p = ApproximateZeroCrossingPosition(p1, p2);
                float3 n = CalculateSurfaceNormal(p);
				
                qef_add(float4(n.x, n.y, n.z, 0), float4(p.x, p.y, p.z, 0), ATA, Atb, pointaccum, btb);
				
                averageNormal += n;
				
                edgeCount++;
            }
        }
		
        averageNormal = normalize(averageNormal / edgeCount);
		
        CornerIndexes[trueIndex] = corners;
		
        float3 com = float3(pointaccum.x, pointaccum.y, pointaccum.z) / pointaccum.w;
        float4 solved_position = float4(0, 0, 0, 0);
		
        if (nodeSize == 1)
        {
            float error = qef_solve(ATA, Atb, pointaccum, solved_position);
			
            float3 Min = nodePos;
            float3 Max = nodePos + float3(1.0f, 1.0f, 1.0f);
            if (solved_position.x < Min.x || solved_position.x > Max.x ||
			    solved_position.y < Min.y || solved_position.y > Max.y ||
			    solved_position.z < Min.z || solved_position.z > Max.z)
            {
                solved_position.x = com.x;
                solved_position.y = com.y;
                solved_position.z = com.z;
            }
        }
        else
        {
            solved_position.x = com.x;
            solved_position.y = com.y;
            solved_position.z = com.z;
        }
		
        Voxels[trueIndex].position = float3(solved_position.x, solved_position.y, solved_position.z);
        Voxels[trueIndex].normal = averageNormal;
        Voxels[trueIndex].numPoints = edgeCount;
    }
}

[numthreads(4, 4, 4)]
void GenerateQuad(int3 id : SV_DispatchThread)
{





}