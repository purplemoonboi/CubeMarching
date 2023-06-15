#include "D3D12Core.h"

namespace Foundation::Graphics::D3D12
{
	HRESULT Init()
	{
		HRESULT hr{ S_OK };



		// Initialise heaps.
		hr = RtvHeap.Init(512, false);
		THROW_ON_FAILURE(hr);
		hr = DsvHeap.Init(512, false);
		THROW_ON_FAILURE(hr);
		hr = SrvHeap.Init(4096, true);
		THROW_ON_FAILURE(hr);
		hr = UavHeap.Init(512, false);
		THROW_ON_FAILURE(hr);

		NAME_D3D12_OBJECT(RtvHeap.GetHeap(), L"RTV Heap Descriptor");
		NAME_D3D12_OBJECT(DsvHeap.GetHeap(), L"DSV Heap Descriptor");
		NAME_D3D12_OBJECT(SrvHeap.GetHeap(), L"SRV Heap Descriptor");
		NAME_D3D12_OBJECT(UavHeap.GetHeap(), L"UAV Heap Descriptor");


#ifdef CM_DEBUG
		ComPtr<ID3D12Debug>  DX12DebugController;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&DX12DebugController));
#endif

		//Create the factory.
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory4));
		THROW_ON_FAILURE(hr);

		//Create the Device.
		hr = D3D12CreateDevice(nullptr, /* Use dedicated GPU.*/ D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));

		//If we were unable to create a Device using our
		//dedicated GPU, try to create one with the WARP
		//Device.
		if (FAILED(hr))
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			HRESULT warpEnumResult = pDXGIFactory4->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
			HRESULT warpDeviceResult = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
		}

	
		return hr;
	}

	void Shutdown()
	{
	}

	void __declspec(noinline) ProcessDeferrals(UINT32 frame)
	{
		std::lock_guard{ DeferralsMutex };

		DeferralFlags[frame] = 0;

		RtvHeap.ProcessDeferredFree(frame);
		DsvHeap.ProcessDeferredFree(frame);
		SrvHeap.ProcessDeferredFree(frame);
		UavHeap.ProcessDeferredFree(frame);

		std::vector<IUnknown*>& resources{ DeferredReleases[frame] };
		if (!resources.empty())
		{
			for (auto& resource : resources)
			{
				Release(resource);
			}
			resources.clear();
		}
	}

	namespace Internal
	{
		void DeferredRelease(IUnknown* resource)
		{
			const UINT32 frame = FrameIndex;
			std::lock_guard{ DeferralsMutex };

			DeferredReleases[frame].push_back(resource);
			SetDeferredReleasesFlag();
		}
	}


}
