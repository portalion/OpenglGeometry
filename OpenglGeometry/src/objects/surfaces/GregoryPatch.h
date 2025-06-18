#pragma once
#include "core/RenderableOnScene.h"
class GregoryPatch : public RenderableOnScene
{
	inline std::string GetTypeName() const { return "Gregory Patch"; }
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

