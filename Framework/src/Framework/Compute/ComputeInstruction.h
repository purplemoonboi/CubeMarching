#pragma once
#include <intsafe.h>

#include "ComputeApi.h"

namespace Engine
{
	class ComputeInstruction
	{
	public:

		
		static void Dispatch(const BYTE* data, INT32 x, INT32 y, INT32 z)
		{
			Compute->Dispatch(data, x, y, z);
		}

		static void ResetComputeCommandList()
		{
			Compute->ResetComputeCommandList();
		}

		static void ExecuteComputeCommandList()
		{
			Compute->ExecuteComputeCommandList();
		}

	private:

		static ComputeApi* Compute;

	};
}
