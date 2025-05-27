#pragma once
#include "core/RenderableOnScene.h"

class AxisCursor : public virtual RenderableOnScene, public virtual PositionComponent
{
	std::string GetTypeName() const;
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	inline bool DisplayParameters() 
	{ 
		return false; 
	};

	inline json Serialize() const override
	{
		return json::object();
	}
};

