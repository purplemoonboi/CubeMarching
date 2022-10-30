#pragma once
#include "Framework/Core/Core.h"

namespace DX12Framework
{
	class MainCamera;

	class Scene
	{

	public:
		Scene() = delete;
		Scene(const std::string& name);

		void OnUpdate(const float deltaTime);
		void OnRender();


		MainCamera* SceneCamera;
	};
}
