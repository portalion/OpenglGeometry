#pragma once
#include "core/RenderableOnScene.h"

class Torus : public RenderableOnScene
{
private:
	float radius = 15.f;
	float tubeRadius = 5.f;
	unsigned int tubeSegments = 50;
	unsigned int radiusSegments = 30;

	Algebra::Vector4 GetPoint(float angleTube, float angleRadius);

	std::string GetTypeName() const override;
	virtual RenderableOnSceneMesh GenerateMesh() override;
	virtual bool DisplayParameters() override;
};

