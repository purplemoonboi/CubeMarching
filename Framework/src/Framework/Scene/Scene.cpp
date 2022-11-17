#include "Framework/cmpch.h"
#include "Scene.h"
#include "Framework/Renderer/Renderer3D.h"



namespace Engine
{
	Scene::Scene(const std::string& name)
	{
		SceneCamera = new MainCamera(1920, 1080);
	}

	void Scene::OnUpdate(const float timer)
	{

		/** process scripts and entities */
		SceneCamera->Update(timer);

	}

	void Scene::OnRender(const float timer)
	{
		/** process drawing instructions */

		Renderer3D::BeginScene(*SceneCamera, timer);

		

		Renderer3D::EndScene();
	}
}
