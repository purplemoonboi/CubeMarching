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
    unsigned int xx = ExpandBits((unsigned int) x);
    unsigned int yy = ExpandBits((unsigned int) y);
    unsigned int zz = ExpandBits((unsigned int) z);
    return xx * 4 + yy * 2 + zz;
}

uint Encode12BitMorton(float3 xyz)
{
    uint xb = ((int) xyz.x) & 0x11110000b;
    uint yb = ((int) xyz.y) & 0x11110000b;
    uint zb = ((int) xyz.z) & 0x11110000b;
    
    uint morton = 0x000000000000;
    
    morton = ((xb & 0x1b) >> 2048) | ((yb & 0x1b) >> 1024) | ((zb & 0x1b) >> 512)
           | ((xb & 0x01b) >> 256) | ((yb & 0x01b) >> 128) | ((zb & 0x01b) >> 64)
           | ((xb & 0x001b) >> 32) | ((yb & 0x001b) >> 16) | (zb & 0x001b);
    
    return morton;
}