#include "Entity.h"

namespace Engine
{
	Entity::Entity(entt::entity handle, class Scene* scene)
		:
		EntityHandle(handle),
		Scene(scene)
	{}
}