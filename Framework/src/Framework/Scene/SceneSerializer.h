#pragma once
#include <string>

#include "Scene.h"
#include "Framework\Core\core.h"

namespace Foundation
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const RefPointer<Scene>& scene);

		// @brief Stores the entire scene as a text file.
		// @param[in] Location where the file is stored.
		void Serialise(const std::string& filepath, const std::string& sceneName = "Untitled");
		void SerialiseRuntime(const std::string& filepath);

		// @brief Loads scene data from a file into the ECS.
		// @param[in] The location of the scene file.
		bool Deserialise(const std::string& filepath);
		bool DeserialiseRuntime(const std::string& filepath);
	private:
		RefPointer<Scene> Scene;
	};
}
