#pragma once
#include "core/Base.h"
#include "scene/Entity.h"
#include <Algebra.h>

class Scene;
struct Tool;

// Wireframe cutter drawn at the tool tip, in path file coordinates (Z up).
class ToolModel
{
private:
	Entity m_Entity;
	float m_Diameter = 0.f;
	bool m_IsFlat = false;
	bool m_Visible = true;

	void Rebuild(const Tool& tool);

public:
	ToolModel(Ref<Scene> scene);

	void Update(const Tool& tool, const Algebra::Vector4& tipPosition);
	void SetVisible(bool visible);
};
