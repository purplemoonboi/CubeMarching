#pragma once
#include <intsafe.h>
#include "DirectX12.h"
#include "Framework/Renderer/RendererAPI.h"

namespace DX12Framework
{

	class DX12GraphicsContext;
	class DX12FrameBuffer;

	class DX12RenderingApi : public RendererAPI
	{
	public:
		virtual ~DX12RenderingApi();


		void Init() override;

		void InitD3D(HWND windowHandle, INT32 viewportWidth, INT32 viewportHeight) override;

		void SetViewport(INT32 x, INT32 y, INT32 width, INT32 height) override;

		void SetClearColour(DirectX::XMFLOAT4 colour) override;

		void Draw() override;

		void Clear() override;

	private:

		DX12GraphicsContext* GraphicsContext = nullptr;

		DX12FrameBuffer* FrameBuffer = nullptr;

	};

}

