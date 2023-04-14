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
    x = min(max(x * 1024.0f, 0.0f), 1023.0f);
    y = min(max(y * 1024.0f, 0.0f), 1023.0f);
    z = min(max(z * 1024.0f, 0.0f), 1023.0f);
    uint xx = ExpandBits((uint) x);
    uint yy = ExpandBits((uint) y);
    uint zz = ExpandBits((uint) z);
    return xx * 4 + yy * 2 + zz;
}

uint EncodeMorton(float3 xyz)
{
    uint code = 0;
    
    uint xx = ((uint)xyz.x & 0x11110000);
    uint yy = ((uint)xyz.y & 0x11110000);
    uint zz = ((uint)xyz.z & 0x11110000);
    
    return 0;
}

