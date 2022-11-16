#pragma once
#include "Buffer.h"
#include "Framework/Core/core.h"

namespace Engine
{
	// Simple vertex buffer
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color;
	};

	struct PassConstants
	{
		DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;
	};

	// Simple constant buffer
	struct ObjectConstant
	{
		DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	};



	class GraphicsContext;

	struct FrameResource
	{
		virtual ~FrameResource() = default;

		static ScopePointer<FrameResource> Create(GraphicsContext* graphicsContex, UINT passCount, UINT objectCount);
	};
}
