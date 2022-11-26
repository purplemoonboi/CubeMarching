#pragma once
#include "DirectX12.h"
#include "Framework/Core/core.h"
#include "Framework/Renderer/FrameResource.h"


#include "DX12UploadBuffer.h"
#include "MathHelper.h"

namespace Engine
{



    // Stores the resources needed for the CPU to build the command lists
	// for a frame.  
    struct DX12FrameResource : FrameResource
    {
        DX12FrameResource(GraphicsContext* graphicsContext, UINT passCount, UINT objectCount, UINT id);
        DX12FrameResource(const DX12FrameResource& rhs) = delete;
        DX12FrameResource& operator=(const DX12FrameResource& rhs) = delete;
        ~DX12FrameResource() override;

        // We cannot reset the allocator until the GPU is done processing the commands.
        // So each frame needs their own allocator.
        ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		// We cannot update a cbuffer until the GPU is done processing the commands
		// that reference it.  So each frame needs their own cbuffers.
		ScopePointer<DX12UploadBuffer<PassConstants>> PassBuffer = nullptr;
		ScopePointer<DX12UploadBuffer<ObjectConstant>> ConstantBuffer = nullptr;

		// Fence value to mark commands up to this fence point.  This lets us
		// check if these frame resources are still in use by the GPU.
		UINT64 Fence = 0;
    };

}
