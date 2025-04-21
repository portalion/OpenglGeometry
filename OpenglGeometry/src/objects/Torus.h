 #pragma once
#include "core/RenderableOnScene.h"
#include <core/transformations/TransformationComponents.h>

class Torus : public RenderableOnScene
{
private:
	PositionComponent positionComponent;
	RotationComponent rotationComponent;
	ScaleComponent scaleComponent;

	float radius = 15.f;
	float tubeRadius = 5.f;
	unsigned int tubeSegments = 50;
	unsigned int radiusSegments = 30;

	Algebra::Vector4 GetPoint(float angleTube, float angleRadius);

	inline std::string GetTypeName() const override { return "Torus"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;
public:
	inline IPositionComponent* GetPositionComponent() override { return &positionComponent; }
	inline IRotationComponent* GetRotationComponent() override { return &rotationComponent; }
	inline IScaleComponent* GetScaleComponent() override { return &scaleComponent; }
};

