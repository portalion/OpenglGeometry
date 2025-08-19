#pragma once
#include "Scene.h"
#include <entt/entt.hpp>
#include <utility>
#include <string>

class Entity
{
private:
	entt::entity m_EntityHandle;
	Scene* m_Scene;
public:
	Entity() = default;
	Entity(entt::entity entity, Scene* m_Scene);

	template<typename T>
	void AddTag()
	{
		if (!this->HasComponent<T>())
		{
			m_Scene->m_Registry.emplace<T>(m_EntityHandle);
		}
	}

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T, typename... Args>
	T& AddOrReplaceComponent(Args&&... args)
	{
		T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	T& GetComponent()
	{
		return m_Scene->m_Registry.get<T>(m_EntityHandle);
	}

	template<typename T>
	bool HasComponent()
	{
		return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		m_Scene->m_Registry.remove<T>(m_EntityHandle);
	}

	template<typename T>
	void RemoveTag()
	{
		this->RemoveComponent<T>();
	}

	std::string GetID() const
	{
		return std::to_string(static_cast<uint32_t>(m_EntityHandle));
	}

	bool IsValid() const
	{
		return m_Scene->m_Registry.valid(m_EntityHandle);
	}

	friend Scene;
};

