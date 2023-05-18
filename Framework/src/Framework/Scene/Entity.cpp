#include "Entity.h"

namespace Foundation
{
	Entity::Entity(entt::entity handle, class Scene* scene)
		:
		EntityHandle(handle),
		Scene(scene)
	{}
}