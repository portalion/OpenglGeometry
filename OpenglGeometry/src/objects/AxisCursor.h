#pragma once
#include "core/RenderableOnScene.h"

class AxisCursor : public RenderableOnScene
{
	std::string GetTypeName() const override;
	RenderableOnSceneMesh GenerateMesh() override;
	inline bool DisplayParameters() override { return false; };
};

