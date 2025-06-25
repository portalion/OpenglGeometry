#pragma once
#include "core/RenderableOnScene.h"

class AxisCursor : public RenderableOnScene
{
private:
	std::string GetTypeName() const;
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	inline bool DisplayParameters() { return false; };

};

