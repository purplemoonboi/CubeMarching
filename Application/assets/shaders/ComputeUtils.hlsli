// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
unsigned int ExpandBits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
unsigned int Morton3D(float x, float y, float z)
{
    x = min(max(x * 32.0f, 0.0f), 31.0f);
    y = min(max(y * 32.0f, 0.0f), 31.0f);
    z = min(max(z * 32.0f, 0.0f), 31.0f);
    uint xx = ExpandBits((uint) x);
    uint yy = ExpandBits((uint) y);
    uint zz = ExpandBits((uint) z);
    return xx * 4 + yy * 2 + zz;
}

float3 IndexTo3DPoint(uint i, uint w, uint h, uint d)
{
    float z = (float)  i % h;
    float y = ((float) i / w) % d;
    float x = (float)  i / (w * d);
    
    float3 p = float3(x, y, z);
    return p;
}

