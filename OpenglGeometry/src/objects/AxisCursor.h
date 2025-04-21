#pragma once
#include "core/RenderableOnScene.h"
#include <core/transformations/TransformationComponents.h>

class AxisCursor : public RenderableOnScene
{
private:
	PositionComponent positionComponent;

	std::string GetTypeName() const;
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	inline bool DisplayParameters() { return false; };

public:
	IPositionComponent* GetPositionComponent() override;
};

