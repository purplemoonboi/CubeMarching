#pragma once
#include "Scene.h"

#include "entt.hpp"

#include "Framework/Core/Log/Log.h"

namespace Engine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;
	

		template<typename T, typename ... Args> T& AddComponent(Args&&... args)
		{
			CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = Scene->Registry.emplace<T>(EntityHandle, std::forward<Args>(args)...);

			Scene->OnComponentAdded<T>(*this, component);

			return component;
		}

		template<typename T> T& GetComponent() const 
		{
			CORE_ASSERT(HasComponent<T>(), "Entity  does not have component!");

			if (HasComponent<T>())
			{
				return Scene->Registry.get<T>(EntityHandle);
			}
		}

		template<typename T> bool HasComponent() const
		{
			return Scene->Registry.any_of<T>(EntityHandle);
		}

		template<typename T> void RemoveComponent() const
		{
			Scene->Registry.remove<T>(EntityHandle);
		}

		operator bool() const { return EntityHandle != entt::null; }

		// @brief Implicit conversion operator to an unsigned 32-bit integer.
		operator UINT32() const { return (UINT32)EntityHandle; }

		// @brief Implicit conversion operator to a entity handle.
		operator entt::entity() const { return EntityHandle; }

		// @brief Comparator operator for a valid entity handle.
		bool operator ==(const Entity& other) const 
		{
			return EntityHandle == other.EntityHandle && Scene == other.Scene;
		}

		// @brief Comparator operator for entity object comparisons.
		bool operator !=(const Entity& other) const
		{
			return !(*this == other);
		}

	private:
		entt::entity EntityHandle{ entt::null };
		Scene* Scene = nullptr;
	};

}


