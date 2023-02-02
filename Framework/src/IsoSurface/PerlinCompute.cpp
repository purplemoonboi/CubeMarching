#include "PerlinCompute.h"


namespace Engine
{
	void PerlinCompute::Init(GraphicsContext* context)
	{
		Context = dynamic_cast<D3D12Context*>(context);

		BuildComputeRootSignature();

		ScalarField = std::make_unique<D3D12Texture>(L" ");
		ScalarField->Create(32, 32, Context, 
			TextureDimension::Three);

		CopyField = std::make_unique<D3D12Texture>(L" ");
		CopyField->Create(32, 32, Context,
			TextureDimension::Three);
	}

	void PerlinCompute::Generate3DTexture(PerlinArgs args)
	{


		// #3 - Dispatch the work onto the GPU.
		Context->GraphicsCmdList->Dispatch(1, 1, 1);

		// #4 - Read back the vertices into the vertex buffer.
		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(ScalarField->Texture.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));

		// Copy vertices from the compute shader into the vertex buffer for rendering
		Context->GraphicsCmdList->CopyResource(CopyField->Texture.Get(), ScalarField->Texture.Get());

		Context->GraphicsCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(ScalarField->Texture.Get(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));


		// Execute the commands and flush
		THROW_ON_FAILURE(Context->GraphicsCmdList->Close());
		ID3D12CommandList* cmdsLists[] = { Context->GraphicsCmdList.Get() };
		Context->CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		Context->FlushCommandQueue();
	}

	void PerlinCompute::BuildComputeRootSignature()
	{

		CD3DX12_DESCRIPTOR_RANGE srvTable;
		srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE uavTable;
		uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[3];

		// Perfomance TIP: Order from most frequent to least frequent.
		slotRootParameter[0].InitAsConstants(5, 0); // world settings view
		slotRootParameter[1].InitAsDescriptorTable(1, &srvTable); // density texture view
		slotRootParameter[2].InitAsDescriptorTable(1, &uavTable);// output buffer view

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature
		(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr) { ::OutputDebugStringA((char*)errorBlob->GetBufferPointer()); }

		THROW_ON_FAILURE(hr);
		THROW_ON_FAILURE(Context->Device->CreateRootSignature
		(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(ComputeRootSignature.GetAddressOf()
			)
		));
	}

	void PerlinCompute::BuildComputeResourceDescriptors()
	{
	}
}

