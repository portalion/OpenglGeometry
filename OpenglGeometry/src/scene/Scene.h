#pragma once
#include <entt/entt.hpp>

class Entity;

template<typename... Exclude>
using Excluded = entt::exclude_t<Exclude...>;

class Scene
{
private:
	entt::registry m_Registry;
	void OnPositionCreated(entt::registry& registry, entt::entity entity);
public:
	Scene();
	~Scene();
	
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	Entity CreateEntity();
	void DestroyEntity(Entity entity);

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

