#pragma once
#include <intsafe.h>
#include "DirectX12.h"
#include "Framework/Core/core.h"

#include "DX12UploadBuffer.h"
#include "MathHelper.h"

namespace Engine
{

	// Simple vertex buffer
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Colour;
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
		DirectX::XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	};


    // Stores the resources needed for the CPU to build the command lists
// for a frame.  
    struct DX12FrameResource
    {
    public:

        DX12FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
        DX12FrameResource(const DX12FrameResource& rhs) = delete;
        DX12FrameResource& operator=(const DX12FrameResource& rhs) = delete;
        ~DX12FrameResource();

        // We cannot reset the allocator until the GPU is done processing the commands.
        // So each frame needs their own allocator.
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

        // We cannot update a cbuffer until the GPU is done processing the commands
        // that reference it.  So each frame needs their own cbuffers.
        RefPointer<DX12UploadBuffer<PassConstants>> PassCB = nullptr;
        RefPointer<DX12UploadBuffer<ObjectConstant>> ObjectCB = nullptr;

        // Fence value to mark commands up to this fence point.  This lets us
        // check if these frame resources are still in use by the GPU.
        UINT64 Fence = 0;
    };
}
