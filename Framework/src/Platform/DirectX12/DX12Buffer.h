#pragma once
#include "Framework/Renderer/Buffer.h"
#include "Framework/Primitives/GeometryBase.h"
#include "DX12UploadBuffer.h"
#include "DirectX12.h"

#include <memory>

namespace Engine
{

	// Using namespace
	using Microsoft::WRL::ComPtr;


	// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
	struct SubmeshGeometry
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;

		// Bounding box of the geometry defined by this submesh. 
		// This is used in later chapters of the book.
		DirectX::BoundingBox Bounds;
	};

	struct MeshGeometry
	{
		// Give it a name so we can look it up by name.
		std::string Name;

		// System memory copies.  Use Blobs because the vertex/index format can be generic.
		// It is up to the client to cast appropriately.  
		ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
		ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

		ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
		ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

		ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
		ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

		// Data about the buffers.
		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
		UINT IndexBufferByteSize = 0;

		// A MeshGeometry may store multiple geometries in one vertex/index buffer.
		// Use this container to define the Submesh geometries so we can draw
		// the Submeshes individually.
		std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
			vbv.StrideInBytes = VertexByteStride;
			vbv.SizeInBytes = VertexBufferByteSize;

			return vbv;
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
			ibv.Format = IndexFormat;
			ibv.SizeInBytes = IndexBufferByteSize;

			return ibv;
		}

		// We can free this memory after we finish upload to the GPU.
		void DisposeUploaders()
		{
			VertexBufferUploader = nullptr;
			IndexBufferUploader = nullptr;
		}
	};



	class DX12VertexBuffer : public VertexBuffer
	{
	public:
		DX12VertexBuffer(INT32 size);
		DX12VertexBuffer(float* vertices, INT32 size);
		virtual ~DX12VertexBuffer() = default;

		// @brief Binds this buffer for modifications.
		void Bind() const override;

		// @brief Releases this buffer.
		void UnBind() const override;

		// @brief Sets the vertex data for this buffer.
		void SetData(const void* data, INT32 size) override;

		inline void SetLayout(const BufferLayout& layout) override;

		const BufferLayout& GetLayout() const override { return Layout; }


		// @brief Returns the view into the vertex buffer;
		D3D12_VERTEX_BUFFER_VIEW GetBufferView() const { return BufferView; }

	private:
		BufferLayout Layout;

		D3D12_VERTEX_BUFFER_VIEW BufferView;
		
	};



	class DX12IndexBuffer : public IndexBuffer
	{
	public:
		DX12IndexBuffer(INT32* indices, INT32 size);

		virtual ~DX12IndexBuffer() = default;

		void Bind() const override;

		void UnBind() const override;

		const INT32 GetCount() override { return Count; }

	private:
		INT32 Count;

	};



	class DX12ConstantBuffer
	{
	public:
		DX12ConstantBuffer() = default;
		virtual ~DX12ConstantBuffer() = default;

		void BuildConstantBuffer
		(
			ID3D12Device* device, 
			D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle,
			UINT count, 
			bool isConstantBuffer = true
		);

		// @brief Defines the resources the shader programs should expect.
		void BuildRootSignature(ID3D12Device* device);

		void BuildShaderInputAndLayout();

		void BuildBoxGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* graphicsCmdList);

		void BuildPSO(ID3D12Device* device, DXGI_FORMAT backbufferFormat, DXGI_FORMAT depthStencilFormat);

	private:
		ComPtr<ID3DBlob> mvsByteCode;
		ComPtr<ID3DBlob> mpsByteCode;
		ComPtr<ID3D12RootSignature> RootSignature;
		ComPtr<ID3D12PipelineState> PipelineState;
		RefPointer<DX12UploadBuffer<ObjectConstant>> ObjectCB = nullptr;

		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
		RefPointer<MeshGeometry> BoxGeo;
	};

}


