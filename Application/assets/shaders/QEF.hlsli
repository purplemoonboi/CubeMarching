#include "ShaderConstants.hlsli"
#include "SVD.hlsli"

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

float QEF_Solve(mat3x3_tri ATA, float4 Atb, float4 pointaccum, inout float x)
{

	float4 massPoint = pointaccum / pointaccum.w;

	float4 A_mp = float4(0, 0, 0, 0);
	SVD_VMul_Sym(A_mp, ATA, masspoint);

	float error = QEF_CalculateError(ATA, x, Atb);
	x += masspoint;

	return error;
}