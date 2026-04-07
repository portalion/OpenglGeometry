#pragma once
#include "interfaces/IPopup.h"
#include <scene/Components.h>
#include <scene/Tags.h>
#include <core/Base.h>
#include <scene/Scene.h>
#include <scene/Entity.h>

class ShapeCreation : public IPopup
{
private:
	Ref<Scene> m_Scene;
	Entity m_Cursor;

	std::vector<Entity> GetSelectedPoints();
public:
	ShapeCreation(Ref<Scene> scene);

	inline const char* Name() override { return "Shape Creation"; };
	bool ShouldOpen() override;
	void Display() override;
};