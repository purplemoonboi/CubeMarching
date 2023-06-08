#pragma once
#include "Framework/Renderer/Resources/Light.h"

using namespace DirectX;


namespace Foundation::Graphics
{

	struct Vertex
	{
		Vertex() {}
		Vertex(
			const DirectX::XMFLOAT3& p,
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT3& t,
			const DirectX::XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexC(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v) {}

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(Vertex& va, Vertex& vb, Vertex& vc)
			:
			VertexA(va),
			VertexB(vb),
			VertexC(vc)
		{}

		Vertex VertexA;
		Vertex VertexB;
		Vertex VertexC;
	};

	struct PassConstants
	{
		XMFLOAT4X4 View				 = MathHelper::Identity4x4();
		XMFLOAT4X4 InvView			 = MathHelper::Identity4x4();
		XMFLOAT4X4 Proj				 = MathHelper::Identity4x4();
		XMFLOAT4X4 InvProj			 = MathHelper::Identity4x4();
		XMFLOAT4X4 ViewProj			 = MathHelper::Identity4x4();
		XMFLOAT4X4 InvViewProj		 = MathHelper::Identity4x4();
		XMFLOAT3 EyePosW			 = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1		 = 0.0f;
		XMFLOAT2 RenderTargetSize	 = { 0.0f, 0.0f };
		XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ					 = 0.0f;
		float FarZ					 = 0.0f;
		float TotalTime				 = 0.0f;
		float DeltaTime				 = 0.0f;
		XMFLOAT4 AmbientLight		 = { 0.0f, 0.0f, 0.0f, 1.0f };
		Light Lights[MaxLights];
	};

	// Simple constant buffer
	struct ObjectConstant
	{
		XMFLOAT4X4 World = MathHelper::Identity4x4();
		XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
		UINT32 MaterialIndex;
		UINT32 ObjPad0;
		UINT32 ObjPad1;
		UINT32 ObjPad2;
	};

}
