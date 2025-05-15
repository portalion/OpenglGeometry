#pragma once
#include "core/RenderableOnScene.h"

class BezierSurface : public RenderableOnScene
{
private:
	
	inline std::string GetTypeName() const override { return "Bezier Surface"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;
};

