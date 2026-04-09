#pragma once
#include "Entity.h"
#include <unordered_map>

using ComponentAction = std::function<void(Entity)>;

class ComponentFunctionRegistry
{
private:
	std::unordered_map<entt::id_type, ComponentAction> m_ComponentToFunctionStorage;

protected:
	template<typename T>
	void RegisterFunction(ComponentAction function)
	{
		m_ComponentToFunctionStorage[entt::type_hash<T>::value()] = function;
	}

public:
	inline void PerformFunctions(Entity entity) 
	{
		for (const auto& component : entity.GetComponentTypes())
		{
			if (!m_ComponentToFunctionStorage.contains(component)) continue;

			m_ComponentToFunctionStorage[component](entity);
		}
	}
};
