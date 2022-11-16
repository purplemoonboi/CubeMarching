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

        //if ((btnState & MK_LBUTTON) != 0)
        //{
        //    // Make each pixel correspond to a quarter of a degree.
        //    float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - LastMousePos.x));
        //    float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - LastMousePos.y));

        //    // Update angles based on input to orbit camera around box.
        //    Theta += dx;
        //    Phi += dy;

        //    // Restrict the angle mPhi.
        //    Phi = MathHelper::Clamp(Phi, 0.1f, MathHelper::Pi - 0.1f);
        //}
        //else if ((btnState & MK_RBUTTON) != 0)
        //{
        //    // Make each pixel correspond to 0.2 unit in the scene.
        //    float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
        //    float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);

        //    // Update the camera radius based on input.
        //    mRadius += dx - dy;

        //    // Restrict the radius.
        //    mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
        //}

        //LastMousePos.x = x;
        //LastMousePos.y = y;

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
