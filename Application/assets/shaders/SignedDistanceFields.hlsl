

Texture3D<float> DensityTexture : register(t0)


float GenerateBox(float3 position, float3 origin)
{
	float3 localPos = position - origin;



	return 0;
}

[numthreads(1, 1, 1)]
void GeneratePrimtive(uint3 dtId: SV_DispatchThreadID )
{



}