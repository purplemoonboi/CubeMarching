
#define SVD_NUM_SWEEPS 4
#define PSUEDO_INVERSE_THRESHOLD 1e-12f

//typedef float float3x3[3][3];
//typedef float mat3x3_tri[6];


void svd_mul_matrix_vec(inout float3 result, float3x3 a, float3 solvedPosition)
{
    result.x = dot(float3(a[0][0], a[0][1], a[0][2]), solvedPosition);
    result.y = dot(float3(a[1][0], a[1][1], a[1][2]), solvedPosition);
    result.z = dot(float3(a[2][0], a[2][1], a[2][2]), solvedPosition);
}

void givens_coeffs_sym(float a_pp, float a_pq, float a_qq, inout float c, inout float s)
{
    if (a_pq == 0.0f)
    {
        c = 1.0f;
        s = 0.0f;
        return;
    }
	
    float tau = (a_qq - a_pp) / (2.0f * a_pq);
    float stt = sqrt(1.0f + tau * tau);
    float tan = 1.0f / ((tau >= 0.0f) ? (tau + stt) : (tau - stt));
    c = rsqrt(1.0f + tan * tan);
    s = tan * c;
}

void svd_rotate_xy(inout float x, inout float y, float c, float s)
{
    float u = x;
    float v = y;
    x = c * u - s * v;
    y = s * u + c * v;
}

void svd_rotateq_xy(inout float x, inout float y, inout float a, float c, float s)
{
    float cc = c * c;
    float ss = s * s;
    float mx = 2.0f * c * s * a;
    float u = x;
    float v = y;
    x = cc * u - mx + ss * v;
    y = ss * u + mx + cc * v;
}

void svd_rotate(inout float3x3 vtav, inout float3x3 v, int a, int b)
{
    if (vtav[a][b] == 0.0f)
        return;
	
    float c, s;
    givens_coeffs_sym(vtav[a][a], vtav[a][b], vtav[b][b], c, s);
	
    float x, y, z;
    x = vtav[a][a];
    y = vtav[b][b];
    z = vtav[a][b];
    svd_rotateq_xy(x, y, z, c, s);
    vtav[a][a] = x;
    vtav[b][b] = y;
    vtav[a][b] = z;
	
    x = vtav[0][3 - b];
    y = vtav[1 - a][2];
    svd_rotate_xy(x, y, c, s);
    vtav[0][3 - b] = x;
    vtav[1 - a][2] = y;
	
    vtav[a][b] = 0.0f;
	
    x = v[0][a];
    y = v[0][b];
    svd_rotate_xy(x, y, c, s);
    v[0][a] = x;
    v[0][b] = y;
	
    x = v[1][a];
    y = v[1][b];
    svd_rotate_xy(x, y, c, s);
    v[1][a] = x;
    v[1][b] = y;
	
    x = v[2][a];
    y = v[2][b];
    svd_rotate_xy(x, y, c, s);
    v[2][a] = x;
    v[2][b] = y;
}

void svd_solve_sym(inout float a[6], inout float4 sigma, float3x3 v)
{
    float3x3 vtav = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
   
    vtav[0][0] = a[0];
    vtav[0][1] = a[1];
    vtav[0][2] = a[2];
    vtav[1][0] = 0.0f;
    vtav[1][1] = a[3];
    vtav[1][2] = a[4];
    vtav[2][0] = 0.0f;
    vtav[2][1] = 0.0f;
    vtav[2][2] = a[5];
    
    for (int i = 0; i < SVD_NUM_SWEEPS; i++)
    {
        svd_rotate(vtav, v, 0, 1);
        svd_rotate(vtav, v, 0, 2);
        svd_rotate(vtav, v, 1, 2);
    }
	
    sigma = float4(vtav[0][0], vtav[1][1], vtav[2][2], 0.0f);
}

float svd_invdet(float x, float tol)
{
    return (abs(x) < tol || abs(1.0f / x) < tol) ? 0.0f : (1.0f / x);
}

void svd_pseudoinverse(inout float3x3 o, float4 sigma, float3x3 v)
{
    float d0 = svd_invdet(sigma.x, PSUEDO_INVERSE_THRESHOLD);
    float d1 = svd_invdet(sigma.y, PSUEDO_INVERSE_THRESHOLD);
    float d2 = svd_invdet(sigma.z, PSUEDO_INVERSE_THRESHOLD);

    o[0][0] = v[0][0] * d0 * v[0][0] + v[0][1] * d1 * v[0][1] + v[0][2] * d2 * v[0][2];
    o[0][1] = v[0][0] * d0 * v[1][0] + v[0][1] * d1 * v[1][1] + v[0][2] * d2 * v[1][2];
    o[0][2] = v[0][0] * d0 * v[2][0] + v[0][1] * d1 * v[2][1] + v[0][2] * d2 * v[2][2];
    
    o[1][0] = v[1][0] * d0 * v[0][0] + v[1][1] * d1 * v[0][1] + v[1][2] * d2 * v[0][2];
    o[1][1] = v[1][0] * d0 * v[1][0] + v[1][1] * d1 * v[1][1] + v[1][2] * d2 * v[1][2];
    o[1][2] = v[1][0] * d0 * v[2][0] + v[1][1] * d1 * v[2][1] + v[1][2] * d2 * v[2][2];
    
    o[2][0] = v[2][0] * d0 * v[0][0] + v[2][1] * d1 * v[0][1] + v[2][2] * d2 * v[0][2];
    o[2][1] = v[2][0] * d0 * v[1][0] + v[2][1] * d1 * v[1][1] + v[2][2] * d2 * v[1][2];
    o[2][2] = v[2][0] * d0 * v[2][0] + v[2][1] * d1 * v[2][1] + v[2][2] * d2 * v[2][2];
}

void svd_solve_ATA_Atb(inout float ATA[6], float3 Atb, inout float3 solvedPosition)
{
    float3x3 V = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	
    float4 sigma = float4(0, 0, 0, 0);
    svd_solve_sym(ATA, sigma, V);
	
    float3x3 Vinv = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    svd_pseudoinverse(Vinv, sigma, V);
    svd_mul_matrix_vec(solvedPosition, Vinv, Atb);
}

void svd_vmul_sym(inout float3 result, float ATA[6], float3 com)
{
    float3 A_row_x = float4(ATA[0], ATA[1], ATA[2], 0);
    result.x = dot(A_row_x, com);
    result.y = ATA[1] * com.x + ATA[3] * com.y + ATA[4] * com.z;
    result.z = ATA[2] * com.x + ATA[4] * com.y + ATA[5] * com.z;
}

// QEF
/////////////////////////////////////////////////

void qef_add
    (
    float3 n, float3 p, 
        inout float ATA[6], inout float3 Atb, 
            inout float4 pointaccum, inout float btb
)
{
    ATA[0] += n.x * n.x;
    ATA[1] += n.x * n.y;
    ATA[2] += n.x * n.z;
    ATA[3] += n.y * n.y;
    ATA[4] += n.y * n.z;
    ATA[5] += n.z * n.z;
	
    float b = dot(p, n);
    Atb.x += n.x * b;
    Atb.y += n.y * b;
    Atb.z += n.z * b;
    btb += b * b;
	
    pointaccum.x += p.x;
    pointaccum.y += p.y;
    pointaccum.z += p.z;
    pointaccum.w += 1.0f;
}

float qef_calc_error(float ATA[6], float3 solvedPosition, float3 Atb)
{
    float3 tmp = (float3) 0;
    svd_vmul_sym(tmp, ATA, solvedPosition);
    tmp = Atb - tmp;
	
    return dot(tmp, tmp);
}

float qef_solve(float ATA[6], float3 Atb, float3 com, inout float3 solvedPosition)
{
	
    float3 A_mp = (float3)0;
    svd_vmul_sym(A_mp, ATA, com);
    
    A_mp = Atb - A_mp;
    svd_solve_ATA_Atb(ATA, A_mp, solvedPosition);
    float error = qef_calc_error(ATA, solvedPosition, Atb);
    solvedPosition += com;
	
    return error;
}

