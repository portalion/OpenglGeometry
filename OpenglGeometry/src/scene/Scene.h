#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include "managers/IdManager.h"

class Entity;

template<typename... Exclude>
using Excluded = entt::exclude_t<Exclude...>;

class Scene
{
private:
	entt::registry m_Registry;
	std::unordered_map<ID, entt::entity> m_EntitiesById;

	void OnPositionCreated(entt::registry& registry, entt::entity entity);
	void OnIdCreated(entt::registry& registry, entt::entity entity);
	void OnIdDestroyed(entt::registry& registry, entt::entity entity);
public:
	Scene();
	~Scene();
	
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	Entity CreateEntity();
	void DestroyEntity(Entity entity);

	void Clear();

	bool HasEntityWithId(ID id) const;
	Entity FindEntityById(ID id);

	template<typename... Components, typename... Exclude>
	auto GetAllEntitiesWith(Excluded<Exclude...> exclude)
	{
		auto view = m_Registry.view<Components...>(exclude);

		return view | std::views::transform([this](entt::entity e)
		{
			return Entity{ e, this };
		});
	}

	template<typename... Components>
	auto GetAllEntitiesWith()
	{
		auto view = m_Registry.view<Components...>();

		return view | std::views::transform([this](entt::entity e) 
		{
			return Entity{ e, this };
		});
	}

	friend Entity;
};
