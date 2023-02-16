#pragma once
#include <intsafe.h>


namespace Engine
{
	class GraphicsContext;

	class ComputeApi
	{
	public:
		virtual ~ComputeApi(){}
		virtual void Init(GraphicsContext* context) = 0;
		virtual void Dispatch(const void* data, INT32 x, INT32 y, INT32 z) = 0;
		virtual void ResetComputeCommandList() = 0;
		virtual void ExecuteComputeCommandList() = 0;


	};
}
