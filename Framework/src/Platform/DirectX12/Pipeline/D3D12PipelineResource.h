#pragma once
#include <entt.hpp>
#include <Framework/Core/Log/Log.h>
#include "Framework/Renderer/Api/FrameResource.h"
#include "Platform/DirectX12/Constants/D3D12GlobalConstants.h"


namespace Foundation
{
	class AppTimeManager;
}

namespace Foundation::Graphics
{
	class Material;
	class MainCamera;

	struct RenderItem;
}

namespace Foundation::Graphics::D3D12
{
	class D3D12FrameResource;

	class D3D12ResourceBuffer
	{
	public:
		D3D12ResourceBuffer() = default;
		DISABLE_COPY_AND_MOVE(D3D12ResourceBuffer);
		~D3D12ResourceBuffer() = default;


		void UpdatePassBuffer(
			D3D12FrameResource* resource,
			const MainCamera* camera,
			AppTimeManager* time,
			bool wireframe
		);

		void UpdateObjectBuffers(D3D12FrameResource* resource, const std::vector<RenderItem*>& renderItems);
		void UpdateSceneObjects(D3D12FrameResource* resource, entt::registry* registry);

	private:
		PassConstants MainPassCB;
	};
}
