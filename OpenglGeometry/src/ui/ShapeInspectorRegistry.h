#pragma once
#include <core/Base.h>
#include <scene/ComponentFunctionRegistry.h>

class ShapeInspectorRegistry : public ComponentFunctionRegistry<>
{
private:
	Ref<Scene> m_Scene;
	void PositionInspect(Entity entity);
	void ScaleInspect(Entity entity);
	void RotationInspect(Entity entity);
	void LineInspect(Entity entity);
	void VirtualInspect(Entity entity);
	void TorusInspect(Entity entity);
public:
	ShapeInspectorRegistry(Ref<Scene> scene);

	void Display();
};