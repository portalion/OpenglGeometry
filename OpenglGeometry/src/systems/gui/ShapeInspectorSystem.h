#pragma once
#include <core/Base.h>
#include <scene/ComponentFunctionRegistry.h>
#include <interfaces/ISystem.h>

class ShapeInspectorSystem : public ComponentFunctionRegistry, public ISystem
{
private:
	Ref<Scene> m_Scene;
	void PositionInspect(Entity entity);
	void ScaleInspect(Entity entity);
	void RotationInspect(Entity entity);
	void LineInspect(Entity entity);
	void VirtualInspect(Entity entity);

protected:
	template<typename TComponent, typename TClass>
	inline void Bind(void(TClass::* memberFunc)(Entity));

public:
	ShapeInspectorSystem(Ref<Scene> scene);

	void Process() override;
};

template<typename TComponent, typename TClass>
inline void ShapeInspectorSystem::Bind(void(TClass::* memberFunc)(Entity))
{
	RegisterFunction<TComponent>([this, memberFunc](Entity e)
		{
			(static_cast<TClass*>(this)->*memberFunc)(e);
		});
}