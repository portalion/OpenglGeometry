#pragma once
#include <core/Base.h>
#include <scene/ComponentFunctionRegistry.h>
#include <interfaces/ISystem.h>

class ShapeInspectorSystem : public ComponentFunctionRegistry<>, public ISystem
{
private:
	Ref<Scene> m_Scene;
	void PositionInspect(Entity entity);
	void ScaleInspect(Entity entity);
	void RotationInspect(Entity entity);
	void LineInspect(Entity entity);
	void VirtualInspect(Entity entity);

public:
	ShapeInspectorSystem(Ref<Scene> scene);

	void Process() override;
};