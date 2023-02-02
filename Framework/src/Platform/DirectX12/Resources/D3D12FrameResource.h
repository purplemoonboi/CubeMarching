#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Framework/Core/core.h"
#include "Framework/Renderer/Api/FrameResource.h"


#include "Platform/DirectX12/Buffers/D3D12UploadBuffer.h"
#include "MathHelper.h"

namespace Engine
{
    // Stores the resources needed for the CPU to build the command lists
	// for a frame.  
    struct D3D12FrameResource : FrameResource
    {
        D3D12FrameResource(GraphicsContext* graphicsContext, UINT passCount, UINT materialBufferCount, UINT objectCount, UINT id);
        D3D12FrameResource(const D3D12FrameResource& rhs) = delete;
        D3D12FrameResource& operator=(const D3D12FrameResource& rhs) = delete;
        ~D3D12FrameResource() override;

        // We cannot reset the allocator until the GPU is done processing the commands.
        // So each frame needs their own allocator.
        ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		// We cannot update a cbuffer until the GPU is done processing the commands
		// that reference it.  So each frame needs their own cbuffers.
		ScopePointer<D3D12UploadBuffer<PassConstants>> PassBuffer = nullptr;
        ScopePointer<D3D12UploadBuffer<MaterialConstants>> MaterialBuffer = nullptr;
		ScopePointer<D3D12UploadBuffer<ObjectConstant>> ConstantBuffer = nullptr;

		// Fence value to mark commands up to this fence point.  This lets us
		// check if these frame resources are still in use by the GPU.
		UINT64 Fence = 0;
    };

}
