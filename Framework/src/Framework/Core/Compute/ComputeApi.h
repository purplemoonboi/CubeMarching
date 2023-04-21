#pragma once
#include <intsafe.h>


namespace Engine
{
	struct FrameResource;
	class GraphicsContext;
	class PipelineStateObject;

	class ComputeApi
	{
	
	public:


		virtual ~ComputeApi(){}
		virtual void Init(GraphicsContext* context) = 0;
		virtual void ResetComputeCommandList(PipelineStateObject* state = nullptr) = 0;
		virtual void ExecuteComputeCommandList(UINT64* gpuSync) = 0;
		virtual void FlushComputeQueue(UINT64* gpuSync) = 0;
		virtual void Wait(UINT64* gpuSync) = 0;
	};
}
