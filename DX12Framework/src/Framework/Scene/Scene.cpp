#include "Framework/cmpch.h"
#include "Scene.h"
#include "Framework/Renderer/Renderer3D.h"



namespace DX12Framework
{
	Scene::Scene(const std::string& name)
	{
		SceneCamera = new MainCamera(1920, 1080);
	}

	void Scene::OnUpdate(const float deltaTime)
	{

		/** process scripts and entities */
		SceneCamera->Update(deltaTime);

	}

	void Scene::OnRender()
	{
		/** process drawing instructions */

		Renderer3D::BeginScene(*SceneCamera);

		Renderer3D::DrawDemoBox();

		Renderer3D::EndScene();
	}
}
