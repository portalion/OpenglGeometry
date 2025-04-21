#pragma once
#include "core/RenderableOnScene.h"
#include <core/transformations/TransformationComponents.h>

class Point : public RenderableOnScene
{
private:
	PositionComponent positionComponent;
	const static RenderableMesh<PositionVertexData> mesh;

	inline std::string GetTypeName() const override { return "Point"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	inline bool DisplayParameters() override { return false; }

public:
	inline Point() { renderingMode = RenderingMode::TRIANGLES; };

	inline IPositionComponent* GetPositionComponent() override { return &positionComponent; }
};

