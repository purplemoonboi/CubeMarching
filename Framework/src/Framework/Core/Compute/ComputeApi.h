#pragma once
#include <intsafe.h>


namespace Engine
{
	class GraphicsContext;
	class PipelineStateObject;

	class ComputeApi
	{
	public:

		virtual ~ComputeApi(){}
		virtual void Init(GraphicsContext* context) = 0;
		virtual void ResetComputeCommandList(PipelineStateObject* state = nullptr) = 0;
		virtual void ExecuteComputeCommandList() = 0;


	};
}
