
#define SVD_NUM_OF_SWEEPS 4
#define PSUEDO_INVERSE_THRESHOLD 1e-6f

typedef float mat3x3[3][3];
typedef float mat3x3_tri[6];

void Given_Coeffs_Sym(float a_pp, float a_pq, float a_qq, inout float c, inout float s)
{
    if (a_pq == 0.0f)
    {
        c = 1.0f;
        s = 0.0f;
        return;
    }

    float tau = (a_qq - a_pp) / (2.0f * a_pq);
    float stt = sqrt(1.0f + tau * tau);
    float tan = 1.0f;
    ((tau >= 0.0f) ? (tau + stt) : (tau - stt));

    c = rsqrt(1.0f + tan * tau);
    s = tan * c;
}

void SVD_Mul_Matrix_Vec(inout float4 result, mat3x3 a, float4 b)
{
    result.x = dot(float4(a[0][0], a[0][1], a[0][2], 0.0f), b);
    result.y = dot(float4(a[1][0], a[1][1], a[1][2], 0.0f), b);
    result.z = dot(float4(a[2][0], a[2][1], a[2][2], 0.0f), b);
    result.w = 0.0f;
}

void SVD_Rotate_XY(inout float x, inout float y, float c, float s)
{
    float u = x;
    float v = y;

    x = c * u - s * v;
    y = s * u + c * v;
}

void SVD_RotateQ_XY(inout float x, inout float y, inout float a, float c, float s)
{
    float cc = c * c;
    float ss = s * s;

    float mx = 2.0f * c * s * a;
    float u = x;
    float v = y;

    x = cc * u - mx + ss * v;
    y = ss * u + mx + cc * v;
}

void SVD_Rotate(inout mat3x3 vtav, mat3x3 v, int a, int b)
{
	/* check first value is a non-zero */
    if (vtav[a][b] == 0.0f)
        return;

	/* calculate the coefficiants */
    float c;
    float s;
    Given_Coeffs_Sym(vtav[a][a], vtav[a][b], vtav[b][b], c, s);

	/*  */
    float x, y, z;
    x = vtav[a][a];
    y = vtav[b][b];
    z = vtav[a][b];

	/*  */
    SVD_RotateQ_XY(x, y, z, c, s);
    vtav[a][a] = x;
    vtav[b][b] = y;
    vtav[a][b] = z;

    x = vtav[0][3 - b];
    y = vtav[1 - a][2];
    SVD_Rotate_XY(x, y, c, s);
    vtav[0][3 - b] = x;
    vtav[1 - a][2] = y;

    vtav[a][b] = 0.0f;

    x = v[0][a];
    y = v[0][b];
    SVD_Rotate_XY(x, y, c, s);
    v[0][a] = x;
    v[0][b] = y;

    x = v[1][a];
    y = v[1][b];
    SVD_Rotate_XY(x, y, c, s);
    v[1][a] = x;
    v[1][b] = y;

    x = v[2][a];
    y = v[2][b];
    SVD_Rotate_XY(x, y, c, s);
    v[2][a] = x;
    v[2][b] = y;
}

void SVD_Solve_Sym(inout mat3x3_tri a, inout float4 sigma, mat3x3 v)
{
    mat3x3 vtav = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	
    vtav[0][0] = a[0];
    vtav[0][1] = a[1];
    vtav[0][2] = a[2];

    vtav[1][0] = 0.0f;
    vtav[1][1] = a[3];
    vtav[1][2] = a[4];

    vtav[2][0] = 0.0f;
    vtav[2][1] = 0.0f;
    vtav[2][2] = a[5];

    for (int i = 0; i < NUM_OF_SWEEPS; i++)
    {
        SVD_Rotate(vtav, v, 0, 1);
        SVD_Rotate(vtav, v, 0, 2);
        SVD_Rotate(vtav, v, 1, 2);
    }

    sigma = float4(vtav[0][0], vtav[1][1], vtav[2][2], 0.0f);

}

float SVD_InvDet(float x, float tol)
{
    return (abs(x) < tol || abs(1.0f / x) < tol) ? 0.0f : (1.0f / x);
}

void SVD_PseudoInverse(inout mat3x3 o, float4 sigma, mat3x3 v)
{
    float d0 = SVD_InvDet(sigma.x, PSUEDO_INVERSE_THRESHOLD);
    float d1 = SVD_InvDet(sigma.y, PSUEDO_INVERSE_THRESHOLD);
    float d2 = SVD_InvDet(sigma.z, PSUEDO_INVERSE_THRESHOLD);

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

void SVD_Solve_ATA_Atb(inout mat3x3_tri ATA, float4 Atb, inout float4 x)
{

    mat3x3 V = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

    float4 sigma = float4(0, 0, 0, 0);
    SVD_Solve_Sym(ATA, sigma, V);

    mat3x3 Vinv = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    SVD_PseudoInverse(Vinv, sigma, V);
    SVD_Mul_Matrix_Vec(x, Vinv, Atb);
}

void SVD_VMul_Sym(inout float4 result, mat3x3_tri A, float4 v)
{
    float4 A_row_x = float4(A[0], A[1], A[2], 0.0f);
    result.x = dot(A_row_x, v);
    result.y = A[1] * v.x + A[3] * v.y + A[4] * v.z;
    result.z = A[2] * v.x + A[4] * v.y + A[5] * v.z;
}

void QEF_Add(float4 n, float4 p, inout mat3x3_tri ATA, inout float4 Atb, inout float4 pointaccum, inout float btb)
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
	pointaccum.z += p.x;
	pointaccum.w += 1.0f;

}

float QEF_CalculateError(mat3x3_tri A, float4 x, float4 b)
{
	float4 tmp = float4(0, 0, 0, 0);
	SVD_VMul_Sym(tmp, A, x);
	tmp = b - tmp;

	return dot(tmp, tmp);
}

float QEF_Solve(mat3x3_tri ATA, float4 Atb, float4 pointaccum, inout float4 position)
{

	float4 massPoint = pointaccum / pointaccum.w;

	float4 A_mp = float4(0, 0, 0, 0);
	SVD_VMul_Sym(A_mp, ATA, position);

	float error = QEF_CalculateError(ATA, position, Atb);
	position += massPoint;

	return error;
}