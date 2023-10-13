#pragma once
#include <intsafe.h>


namespace Foundation::Graphics
{
	class GraphicsContext;
	class RenderPipeline;

	class ComputeApi
	{
	public:
		virtual ~ComputeApi() {}
		virtual void Init(GraphicsContext* context) = 0;
		virtual void ResetComputeCommandList(RenderPipeline* state = nullptr) = 0;
		virtual void ExecuteComputeCommandList(UINT64* gpuSync) = 0;
		virtual void FlushComputeQueue(UINT64* gpuSync) = 0;
		virtual void GlobalSignal(UINT64* gpuSync) = 0;
		virtual void Wait(UINT64* gpuSync) = 0;
	};
}
