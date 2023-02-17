#pragma once
#include <intsafe.h>

#include "ComputeApi.h"

namespace Engine
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

		static void ExecuteComputeCommandList()
		{
			Compute->ExecuteComputeCommandList();
		}

		static ComputeApi* GetComputeApi()
		{
			return Compute;
		}

	private:

		static ComputeApi* Compute;

	};
}
