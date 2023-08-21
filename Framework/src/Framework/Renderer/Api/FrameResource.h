#pragma once
#include "MathHelper.h"


using namespace DirectX;


#define MAX_LIGHTS 16

namespace Foundation::Graphics
{

	struct Vertex
	{
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

	struct Light
	{
		
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
		Light Lights[MAX_LIGHTS];
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
