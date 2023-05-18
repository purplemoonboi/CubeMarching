#pragma once


namespace Foundation
{
	class ImGuiApi
	{
	public:
		virtual void InitialiseImGui() = 0;
		virtual void OnRenderImpl() = 0;
		virtual void EndRenderImpl() = 0;
		virtual void CleanUp() = 0;
	};
}