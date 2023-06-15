#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Framework/Core/core.h"
#include "Framework/Renderer/Api/FrameResource.h"


#include "Platform/DirectX12/UploadBuffer/D3D12UploadBuffer.h"
#include "MathHelper.h"
#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"

namespace Foundation::Graphics::D3D12
{
    // Stores the resources needed for the CPU to build the command lists
	// for a frame.  
    struct D3D12FrameResource
    {
        D3D12FrameResource();
        D3D12FrameResource(UINT passCount, UINT objectCount);
        DISABLE_COPY_AND_MOVE(D3D12FrameResource);

        ~D3D12FrameResource();


        ComPtr<ID3D12GraphicsCommandList4> pGCL{nullptr};
        ComPtr<ID3D12CommandAllocator> pCmdAlloc{nullptr};

		
        D3D12UploadBuffer<PassConstants> PassBuffer;
        D3D12UploadBuffer<ObjectConstant> ConstantBuffer;

        std::array<D3D12RenderTarget, GBUFFER_SIZE> GBuffer;

		UINT64 Fence = 0;
    };

}
