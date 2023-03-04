/*
*   Simple helper functions for generating density primitives
*/
static float Box(float3 worldPosition, float3 origin, float3 halfDimensions)
{
    float3 local_pos = worldPosition - origin;
    float3 pos = local_pos;

    float3 d = float3(abs(pos.x), abs(pos.y), abs(pos.z)) - halfDimensions;
    float m = max(d.x, max(d.y, d.z));
    return min(m, length(max(d, float3(0, 0, 0))));
}

static float Sphere(float3 worldPosition, float3 origin, float radius)
{
    return length(worldPosition - origin) - radius;
}

static float Cylinder(float3 worldPosition, float3 origin, float3 size)
{
    float3 p = worldPosition - origin;
	
    if (Box(worldPosition, origin, size) > 0.0f)
        return 1;
	
    float sqr_dist = (p.x * p.x + p.z * p.z);
    float sqr_rad = size.x * size.x;
    return sqr_dist - sqr_rad;
}

static float Torus(float3 worldPosition, float3 origin)
{
    float3 local_pos = worldPosition - origin;
    float xt = local_pos.x;
    float yt = local_pos.y;
    float zt = local_pos.z;
    float _radius = 10.0f;
    float _radius2 = 2.33f;

    float x = xt;
    float y = yt;
    float z = zt;

    float x2 = sqrt(x * x + z * z) - _radius / 2.0f;
    float d = x2 * x2 + y * y - _radius2 * _radius2;

    return d;
}