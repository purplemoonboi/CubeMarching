#include "DX12Buffer.h"
#include "DX12Buffer.h"

namespace DX12Framework
{
	DX12VertexBuffer::DX12VertexBuffer(INT32 size)
	{
	}

	DX12VertexBuffer::DX12VertexBuffer(float* vertices, INT32 size)
	{



	}

	//Vertex buffer methods
	void DX12VertexBuffer::Bind() const
	{

	}

	void DX12VertexBuffer::UnBind() const
	{

	}

	void DX12VertexBuffer::SetData(const void* data, INT32 size)
	{

	}

	void DX12VertexBuffer::SetLayout(const BufferLayout& layout)
	{

	}

	//Index buffer methods
	void DX12IndexBuffer::Bind() const
	{

	}

	void DX12IndexBuffer::UnBind() const
	{

	}

	void DX12ConstantBuffer::BuildConstantBuffer
	(
		ID3D12Device* device, 
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle,
		UINT count, 
		bool isConstantBuffer
	)
	{

		ObjectCB = CreateRef<DX12UploadBuffer<ObjectConstant>>(device, count, isConstantBuffer);
		
		UINT objectCBBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		// Get the mapped virtual address located on the GPU
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = ObjectCB->Resource()->GetGPUVirtualAddress();

		// Offset to the ith object in the constant buffer
		INT32 boxCBufferIndex = 0;
		cbAddress += boxCBufferIndex * objectCBBytes;


		// Create a constant buffer view
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = DX12BufferUtils::CalculateConstantBufferByteSize(sizeof(ObjectConstant));

		device->CreateConstantBufferView
		(
			&cbvDesc,
			cbvHandle
		);

	}

	void DX12ConstantBuffer::BuildRootSignature(ID3D12Device* device)
	{
		// Root parameter can be a table, root descriptors or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		// Create a single descriptor table of CRVs
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc
		(
			1, 
			slotRootParameter, 
			0, 
			nullptr, 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		ComPtr<ID3DBlob> serialisedRootSignature = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1, 
			serialisedRootSignature.GetAddressOf(), 
			errorBlob.GetAddressOf()
		);

		if(errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		THROW_ON_FAILURE(hr);

		THROW_ON_FAILURE(device->CreateRootSignature
		(
			0,
			serialisedRootSignature->GetBufferPointer(),
			serialisedRootSignature->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature)
		));

	}

	void DX12ConstantBuffer::BuildShaderInputAndLayout()
	{
		HRESULT hr = S_OK;

		mvsByteCode = 


	}
}
