#pragma once
#include <entt/entt.hpp>

class Entity;

class Scene
{
private:
	entt::registry m_Registry;
public:
	Scene() = default;
	~Scene() = default;
	
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	Entity CreateEntity();
	void DestroyEntity(Entity entity);

	template<typename... Components>
	auto GetAllEntitiesWith()
	{
		return m_Registry.view<Components...>();
	}

	friend Entity;
};

