#pragma once
#include <optional>
#include <string>
#include <vector>
#include "UiModel.h"

struct UiState
{
	std::vector<ObjectRow> objects;

	CursorState    cursor;
	PivotMode      pivot = PivotMode::Origin;
	StereoSettings stereo;

	bool showGrid = true;
	bool showControlNets = true;
	bool showVirtualPoints = false;
	bool showSelectionCentre = true;

	std::optional<uint32_t> activeCurveId;
	std::string             statusMessage;

	std::optional<TransformValues> transform;
	std::optional<TorusValues>     torus;
	std::optional<CurveValues>     curve;
	std::optional<SurfaceValues>   surface;

	std::size_t SelectedCount() const;
	std::vector<const ObjectRow*> Selected() const;
	const ObjectRow* Find(uint32_t id) const;

	uint32_t AppendObject(ObjectType type, const std::string& namePrefix);
};
