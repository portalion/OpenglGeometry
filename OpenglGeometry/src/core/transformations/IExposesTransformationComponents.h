#pragma once
#include "ITransformationComponents.h"

class IExposesTransformationComponents
{
public:
	virtual ~IExposesTransformationComponents() = default;
	
	virtual Algebra::Matrix4 GetModelMatrix() = 0;
	
	virtual IPositionComponent* GetPositionComponent() = 0;
	virtual IRotationComponent* GetRotationComponent() = 0;
	virtual IScaleComponent* GetScaleComponent() = 0;
};
