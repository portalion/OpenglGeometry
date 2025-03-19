#pragma once
#include "core/RenderableOnScene.h"

class Point : public RenderableOnScene
{
	const static RenderableOnSceneMesh mesh;

	inline std::string GetTypeName() const override { return "Point"; }
	RenderableOnSceneMesh GenerateMesh() override;
	inline bool DisplayParameters() override { return false; }

public:
	inline Point() { renderingMode = RenderingMode::TRIANGLES; };
};

