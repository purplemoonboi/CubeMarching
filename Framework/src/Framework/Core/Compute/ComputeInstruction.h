#pragma once
#include <intsafe.h>

#include "ComputeApi.h"

namespace Foundation::Compute
{
	class ComputeInstruction
	{
	public:

		static void Init(GraphicsContext* context)
		{
			Compute->Init(context);
		}

		static void ResetComputeCommandList()
		{
			Compute->ResetComputeCommandList();
		}

		static void ExecuteComputeCommandList(UINT64* gpuSync)
		{
			Compute->ExecuteComputeCommandList(gpuSync);
		}

		static ComputeApi* GetComputeApi()
		{
			return Compute;
		}


	private:

		static ComputeApi* Compute;

	};
}
