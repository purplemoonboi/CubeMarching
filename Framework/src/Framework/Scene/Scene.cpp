#include "Framework/cmpch.h"
#include "Scene.h"
#include "Framework/Renderer/Engine/Renderer3D.h"


namespace Engine
{
	Scene::Scene(const std::string& name)
	{
		SceneCamera = new MainCamera(1920, 1080);
	}

	void Scene::OnUpdate(const float deltaTime, const float elapsedTime)
	{

		/** process scripts and entities */
		SceneCamera->Update(deltaTime);

	}

	void Scene::OnRender(const float deltaTime, const float elapsedTime, bool wireframe) const
	{
		/** process drawing instructions */

		Renderer3D::BeginScene(*SceneCamera, deltaTime, wireframe, elapsedTime);

		

		Renderer3D::EndScene();
	}
}
