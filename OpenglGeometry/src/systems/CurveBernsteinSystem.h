#pragma once
#include <vector>
#include "Algebra.h"
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

class CurveBernsteinSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;

	Entity CreateBernsteinPoint(Entity curve, const Algebra::Vector4& position);
	void Resize(Entity curve, std::size_t wanted);
	bool PushDraggedPointsBack(Entity curve, const std::vector<Entity>& deBoorPoints);
	void Recompute(Entity curve);
	void ApplyVisibility(Entity curve);

public:
	CurveBernsteinSystem(Ref<Scene> scene);

	void Process() override;
};
