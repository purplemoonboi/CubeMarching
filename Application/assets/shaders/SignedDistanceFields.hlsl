

Texture3D<float> DensityTexture : register(t0)


float Box(float3 positionW, float3 origin, float3 halfDimensions)
{
	float3 posL = positionW - origin;

	float3 d = float3(abs(posL.x), abs(posL.y), abs(posL.z)) - halfDimensions;
	float m = max(d.x, max(d.y, d.z));

	return min(m, length(max(d, float3(0,0,0))));
}

float Sphere(float3 positionW, float3 origin, float radius)
{
	return length(positionW - origin) - radius;
}

float Cylinder(float3 positionW, float3 origin, float3 size)
{
	float3 p = positionW - origin;
	if (Box(positionW, origin, size) > 0.0f)
	{
		return 1;
	}

	return (p.x * p.x + p.z * p.z) - (p.x * p.x);
}

float Torus(float3 positionW, float3 origin, float radius, float minorRadius)
{

	float3 posL = positionW - origin;
	float xt = posL.x;
	float yt = posL.y;
	float zt = posL.z;

	float x2 = sqrt(x * x + z * z) - radius / 2.0f;
	float d = x2 * x2 + y * y - minorRadius * minorRadius;
	return d;

}

[numthreads(1, 1, 1)]
void GeneratePrimtive(uint3 dtId: SV_DispatchThreadID )
{



}