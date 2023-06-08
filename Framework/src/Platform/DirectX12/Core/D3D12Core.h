#pragma once
#include "Platform/DirectX12/DirectX12.h"
#include "Platform/DirectX12/Constants/D3D12GlobalConstants.h"
#include "Platform/DirectX12/Heap/D3D12HeapManager.h"
#include <array>

#ifdef CM_DEBUG
#define NAME_D3D12_OBJECT(O, name)\
	O->SetName(name);
#else 
#define NAME_D3D12_OBJECT(O, name)
#endif

namespace Foundation::Graphics::D3D12
{

	// Global variables
	inline ComPtr<ID3D12Device8>					pDevice{ nullptr };
	inline ComPtr<IDXGIFactory4>				pDXGIFactory4{ nullptr };
	inline UINT32 FrameIndex{ 0 };

	inline UINT32 DeferralFlags[FRAMES_IN_FLIGHT];
	inline std::mutex DeferralsMutex;


	inline D3D12DescriptorHeap	RtvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
	inline D3D12DescriptorHeap	DsvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
	inline D3D12DescriptorHeap	SrvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
	inline D3D12DescriptorHeap	UavHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

	// @Unsigned integer representing the supported multi sampling quality.
	inline UINT32 MsaaQaulity = 0;
	inline bool MsaaState = false;

	inline std::vector<IUnknown*> DeferredReleases[FRAMES_IN_FLIGHT]{};

	// @brief Checks the MSAA qaulity support and caches level.
	inline void CacheMSAAQuality()
	{
		
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;

		HRESULT hr{ S_OK };

		hr = pDevice->CheckFeatureSupport
		(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msaaQualityLevels,
			sizeof(msaaQualityLevels)
		);
		THROW_ON_FAILURE(hr);

		MsaaQaulity = msaaQualityLevels.NumQualityLevels;
		CORE_ASSERT(MsaaQaulity > 0 && "Unexpected MSAA quality level.", "Unexpected MSAA quality level.");
	}

	

	// Global functions
	namespace Internal
	{
		void DeferredRelease(IUnknown* resource);
	}

	inline void SetDeferredReleasesFlag() { DeferralFlags[FrameIndex] = 1; }

	template<typename T> constexpr void Release(T*& resource)
	{
		if(resource)
		{
			resource->Release();
			resource = nullptr;
		}
	}

	template<typename T> constexpr void DeferredRelease(T*& resource)
	{
		if (resource)
		{
			Internal::DeferredRelease(resource);
			resource = nullptr;
		}
	}

	inline std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
	{


		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
			0.0f,                             // mipLODBias
			8);                               // maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
			0.0f,                              // mipLODBias
			8);                                // maxAnisotropy

		return
		{
			pointWrap, pointClamp,
			linearWrap, linearClamp,
			anisotropicWrap, anisotropicClamp
		};
	}

}