#include "D3D12Core.h"

namespace Foundation::Graphics::D3D12
{

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
