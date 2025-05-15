#pragma once
#include "core/RenderableOnScene.h"
class BezierSurfaceC2 : public RenderableOnScene
{
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	inline std::string GetTypeName() const override { return "Bezier Surface C2"; }
	bool DisplayParameters() override;
};

