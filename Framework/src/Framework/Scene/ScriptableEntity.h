#pragma once
#include "Entity.h"

namespace Foundation
{
	class ScriptableEntity
	{
	public:

		friend class Scene;

	public:
		virtual ~ScriptableEntity() = default;

		template<typename T>
		T& GetComponent()
		{
			return entity.GetComponent<T>();
		}
	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(DeltaTime delta_time) {}
	private:
		Entity entity;
	};
}