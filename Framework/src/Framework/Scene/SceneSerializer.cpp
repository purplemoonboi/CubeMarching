#include "SceneSerializer.h"

#include "Framework/Core/Log/Log.h"

#include "Entity.h"
#include "Components.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Platform/DirectX12/DirectX12.h"

//Following YAML code standards.
namespace YAML
{
	template<>
	struct convert<DirectX::XMFLOAT3>
	{
		static Node encode(const DirectX::XMFLOAT3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<DirectX::XMFLOAT4>
	{
		static Node encode(const DirectX::XMFLOAT4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Foundation
{
	//YAML - Operator Overload: <<
	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT3& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT4& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << vector.w << YAML::EndSeq;
		return out;
	}


	SceneSerializer::SceneSerializer(const RefPointer<class Scene>& scene)
		:
		Scene(scene)
	{}

	// @brief Stores a description of an entity in plain text.
	// @param[in] A special object used to convert data into a text format.
	// @param[in] The entity which is to be serialised.
	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;  //Entity.
		out << YAML::Key << "Entity" << YAML::Value << "192840248"; //TODO: Entity ID goes here.

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			const std::string& tag = entity.GetComponent<TagComponent>().tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			XMFLOAT3& translation = entity.GetComponent<TransformComponent>().Translation;
			XMFLOAT3& rotation = entity.GetComponent<TransformComponent>().Rotation;
			XMFLOAT3& scale = entity.GetComponent<TransformComponent>().Scale;
			out << YAML::Key << "Translation" << YAML::Value << translation;
			out << YAML::Key << "Rotation" << YAML::Value << rotation;
			out << YAML::Key << "Scale" << YAML::Value << scale;
			out << YAML::EndMap;

		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;//Camera component

			CameraComponent& component = entity.GetComponent<CameraComponent>();
			MainCamera& camera = component.Camera;

			out << YAML::Key << "Camera" << YAML::Value << YAML::Value;
			out << YAML::BeginMap;//Camera
			out << YAML::Key << "ProjectionType"   << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV"   << YAML::Value << camera.GetPerspectiveFOV();
			out << YAML::Key << "PerspectiveNear"  << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar"   << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar"  << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;//Camera
			out << YAML::Key << "Primary" << YAML::Value << component.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << component.FixedAspectRatio;
			out << YAML::EndMap;//Camera component
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;//SpriteRendererComponent
			SpriteRendererComponent& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Value << "Colour" << YAML::Value << spriteRendererComponent.Colour;
			out << YAML::EndMap;
		}

		out << YAML::EndMap;	//Entity.
	}

	void SceneSerializer::Serialise(const std::string& filepath, const std::string& sceneName)
	{
		//Create a YAML emitter object
		YAML::Emitter out;

		//The YAML file is essentially a map data structure.
		out << YAML::BeginMap;
		//The file consists of a 'key' and 'value' pattern.
		out << YAML::Key << "Scene" << YAML::Value << sceneName;
		//We store the list of entities in the scene as a sequence.
		//A sequence is an array of values.
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		Scene->Registry.each([&](auto entityID) 
		{
				Entity entity = { entityID, Scene.get() };
				if (!entity)
				{
					return;
				}

				SerializeEntity(out, entity);
		});
		//We then end the sequence of values.
		out << YAML::EndSeq;
		//And close the map off.
		out << YAML::EndMap;

		//Then simply write the structure to an output file.
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerialiseRuntime(const std::string& filepath)
	{
		CORE_ASSERT(false, "Fail");
	}

	// @brief Can't load in scene for some reason.
	bool SceneSerializer::Deserialise(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream stringStream;
		stringStream << stream.rdbuf();

		YAML::Node data = YAML::Load(stringStream.str());
		
		if (!data.IsDefined())
		{
			return false;
		}

		std::string sceneName = data["Scene"].as<std::string>();
		CORE_TRACE("Deserializing scene '{0}'", sceneName);

		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (YAML::Node entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				YAML::Node tagComponentNode = entity["TagComponent"];
				if (tagComponentNode)
				{
					name = tagComponentNode["Tag"].as<std::string>();
				}
				CORE_TRACE("Deserialised entity with ID = {0}, name = {1}", uuid, name);

				//Create a new entity from deserialised data.
				Entity deserialisedEntity = Scene->CreateEntity(name);

				//Get the transform component.
				//NOTE: All entities have a transform component hence 'GetComponent(...)'.
				YAML::Node transformComponentNode = entity["TransformComponent"];
				if (transformComponentNode)
				{
					TransformComponent& transformComponent = deserialisedEntity.GetComponent<TransformComponent>();
					transformComponent.Translation = transformComponentNode["Translation"].as<XMFLOAT3>();
					transformComponent.Rotation = transformComponentNode["Rotation"].as<XMFLOAT3>();
					transformComponent.Scale = transformComponentNode["Scale"].as<XMFLOAT3>();
				}

				//Get the camera component.
				//NOTE: From hereafter we use 'AddComponent(...)'.
				YAML::Node cameraComponentNode = entity["CameraComponent"];

				if (cameraComponentNode)
				{
					CameraComponent cameraComponent = deserialisedEntity.AddComponent<CameraComponent>();

					YAML::Node cameraNode = cameraComponentNode["Camera"];

					cameraComponent.Camera.SetProjectionType((MainCamera::ProjectionType)cameraNode["ProjectionType"].as<int>());

					cameraComponent.Camera.SetPerspectiveFOV(cameraNode["PerspectiveFOV"].as<float>());
					cameraComponent.Camera.SetPerspectiveNearClip(cameraNode["PerspectiveNear"].as<float>());
					cameraComponent.Camera.SetPerspectiveFarClip(cameraNode["PerspectiveFar"].as<float>());

					cameraComponent.Camera.SetOrthographicSize(cameraNode["OrthographicSize"].as<float>());
					const float nc = cameraNode["OrthographicNear"].as<float>();
					cameraComponent.Camera.SetOrthographicNearClip(nc);
					cameraComponent.Camera.SetOrthographicFarClip(cameraNode["OrthographicFar"].as<float>());

					cameraComponent.Primary = cameraComponentNode["Primary"].as<bool>();
					cameraComponent.FixedAspectRatio = cameraComponentNode["FixedAspectRatio"].as<bool>();
				}

				//Get the sprite renderer component.
				YAML::Node spriteRendererComponentNode = entity["SpriteRendererComponent"];
				if (spriteRendererComponentNode)
				{
					SpriteRendererComponent spriteRendererComponent = deserialisedEntity.AddComponent<SpriteRendererComponent>();
					spriteRendererComponent.Colour = spriteRendererComponentNode["Colour"].as<XMFLOAT4>();
				}
			}
		}


		return true;
	}

	bool SceneSerializer::DeserialiseRuntime(const std::string& filepath)
	{
		CORE_ASSERT(false, "Fail");
		return false;
	}
}

