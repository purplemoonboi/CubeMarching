#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Framework/Core/core.h"
#include "Framework/Renderer/Api/FrameResource.h"


#include "Platform/DirectX12/Buffers/D3D12UploadBuffer.h"
#include "MathHelper.h"
#include "Platform/DirectX12/Textures/D3D12RenderTarget.h"

namespace Engine
{
    // Stores the resources needed for the CPU to build the command lists
	// for a frame.  
    struct D3D12FrameResource
    {
        D3D12FrameResource(GraphicsContext* graphicsContext, UINT passCount, UINT materialBufferCount, UINT objectCount, UINT voxelBufferElementCount);
        D3D12FrameResource(const D3D12FrameResource& rhs) = delete;
        D3D12FrameResource& operator=(const D3D12FrameResource& rhs) = delete;
        D3D12FrameResource&& operator=(D3D12FrameResource&& rhs) = delete;
        ~D3D12FrameResource();

        bool QueryTerrainBuffer(UINT elementCount);
        void UpdateVoxelBuffer(const D3D12Context* context, UINT elementCount);

        ComPtr<ID3D12GraphicsCommandList> pGCL;

        ComPtr<ID3D12CommandAllocator> pCA;

		
		ScopePointer<D3D12UploadBuffer<PassConstants>> PassBuffer = nullptr;
        ScopePointer<D3D12UploadBuffer<MaterialConstants>> MaterialBuffer = nullptr;
		ScopePointer<D3D12UploadBuffer<ObjectConstant>> ConstantBuffer = nullptr;
        ScopePointer<D3D12UploadBuffer<Vertex>> TerrainBuffer = nullptr;
        ScopePointer<D3D12RenderTarget> RenderTarget = nullptr;

		UINT64 Fence = 0;
    };

}
