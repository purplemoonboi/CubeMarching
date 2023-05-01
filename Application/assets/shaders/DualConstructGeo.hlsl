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

[numthreads(1, 1, 1)]
void GenerateTriangle(uint3 id : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{

   /*   
    * @brief - In this pass, we check each in parallel for a sign change.
    *          For a sign change along an edge, we connect the voxels adjacent
    *          to this edge.
    *          Care needs to be taken with winding order. i.e a sign change of
    *          '+' -> '-' requires reverse winding order '-' -> '+'
    *   
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
    
     if(forward.z < Resolution)
     {
        /* check the z-axis */
        if (DensityTexture[coord] > IsoLevel && DensityTexture[forward] < IsoLevel)
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
    
      
        if (DensityTexture[coord] < IsoLevel && DensityTexture[forward] > IsoLevel)
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
     }
    
    
    if(right.x < Resolution)
    {
         /* check the x-axes */
        if (DensityTexture[coord] > IsoLevel && DensityTexture[right] < IsoLevel)
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
    
        if (DensityTexture[coord] < IsoLevel && DensityTexture[right] > IsoLevel)
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
    }
   

    if(up.y < Resolution)
    {
        /* check the y-axis */
    
        if (DensityTexture[coord] > IsoLevel && DensityTexture[up] < IsoLevel)
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
    
    
        if (DensityTexture[coord] < IsoLevel && DensityTexture[up] > IsoLevel)
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
    

}