#pragma once
#include "Framework/Renderer/Buffer.h"
#include "DX12UploadBuffer.h"
#include "DirectX12.h"

#include <memory>

namespace DX12Framework
{

	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Colour;
	};

	struct ObjectConstant
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
	};

	// Using namespace
	using Microsoft::WRL::ComPtr;

	class DX12VertexBuffer : public VertexBuffer
	{
		DX12VertexBuffer(INT32 size);
		DX12VertexBuffer(float* vertices, INT32 size);
		
	public:
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
		DX12IndexBuffer(INT32* indices, INT32 size);

	public:
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

	private:
		ComPtr<ID3DBlob> mvsByteCode;
		ComPtr<ID3DBlob> mpsByteCode;
		ComPtr<ID3D12RootSignature> RootSignature;
		RefPointer<DX12UploadBuffer<ObjectConstant>> ObjectCB = nullptr;
	};




}


