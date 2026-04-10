#pragma once
#include "Entity.h"
#include <unordered_map>

template<typename... Args>
class ComponentFunctionRegistry
{
	using ComponentAction = std::function<void(Entity, Args...)>;
private:
	std::unordered_map<entt::id_type, ComponentAction> m_ComponentToFunctionStorage;
	
	template<typename T>
	inline void RegisterFunction(ComponentAction function);
protected:
	template<typename TComponent, typename TClass>
	inline void Bind(void(TClass::* memberFunc)(Entity, Args...));
public:
	inline void PerformFunctions(Entity entity, Args... args);
};


template<typename ...Args>
template<typename T>
inline void ComponentFunctionRegistry<Args...>::RegisterFunction(ComponentAction function)
{
	m_ComponentToFunctionStorage[entt::type_hash<T>::value()] = function;
}

template<typename ...Args>
template<typename TComponent, typename TClass>
inline void ComponentFunctionRegistry<Args...>::Bind(void(TClass::* memberFunc)(Entity, Args...))
{
	RegisterFunction<TComponent>([this, memberFunc](Entity e, Args... args)
		{
			(static_cast<TClass*>(this)->*memberFunc)(e, std::forward<Args>(args)...);
		});
}

template<typename ...Args>
inline void ComponentFunctionRegistry<Args...>::PerformFunctions(Entity entity, Args ...args)
{
	for (const auto& component : entity.GetComponentTypes())
	{
		if (!m_ComponentToFunctionStorage.contains(component)) continue;

		m_ComponentToFunctionStorage[component](entity, args...);
	}
}
