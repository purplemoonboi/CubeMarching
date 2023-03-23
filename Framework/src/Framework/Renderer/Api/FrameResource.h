#pragma once
#include "Framework/Renderer/Buffers/Buffer.h"
#include "Framework/Renderer/Resources/Light.h"
#include "Framework/Core/core.h"

namespace Engine
{
	// Simple vertex buffer
	struct Vertex
	{
		XMFLOAT3 Position	= {0,0,0};
		XMFLOAT3 Normal		= {0,0,0};
		XMFLOAT3 Tangent	= {0,0,0};
		XMFLOAT2 TexCoords	= {0,0};
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
