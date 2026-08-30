#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "Algebra.h"
#include "core/ObjectType.h"

struct TransformValues
{
	Algebra::Vector4 position = Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
	Algebra::Vector4 rotationEuler = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
	Algebra::Vector4 scale = Algebra::Vector4(1.f, 1.f, 1.f, 0.f);
};

struct TorusValues
{
	float    largeRadius = 1.f;
	float    tubeRadius = 0.2f;
	uint32_t samplesU = 16;
	uint32_t samplesV = 32;
};

struct SurfaceValues
{
	uint32_t sizeU = 0;
	uint32_t sizeV = 0;
	uint32_t samplesU = 4;
	uint32_t samplesV = 4;
	uint32_t controlPointCount = 0;
	bool     showControlNet = true;
	bool     selectPointsRequested = false;
};

struct CurveValues
{
	bool     isC2 = false;
	uint32_t controlPointCount = 0;
	bool     showControlPolygon = true;
	bool     showBernsteinPoints = false;
	bool     selectPointsRequested = false;
};

struct ObjectRow
{
	uint32_t    id = 0;
	std::string name;
	ObjectType  type = ObjectType::Point;
	bool        selected = false;
	bool        visible = true;

	std::optional<TransformValues> transform;
	std::optional<TorusValues>     torus;
	std::optional<SurfaceValues>   surface;
	std::optional<CurveValues>     curve;
};

enum class PivotMode
{
	Origin,
	Centre,
	Cursor,
};

inline constexpr const char* ToDisplayString(PivotMode mode)
{
	switch (mode)
	{
	case PivotMode::Origin: return "Origin";
	case PivotMode::Centre: return "Centre";
	case PivotMode::Cursor: return "Cursor";
	}

	return "Origin";
}

struct CursorState
{
	Algebra::Vector4 world = Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
	float            screenX = 0.f;
	float            screenY = 0.f;
	bool             snapToNearest = false;
	Algebra::Vector4 selectionCentre = Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
};

struct StereoSettings
{
	bool  enabled = false;
	float eyeDistance = 0.1f;
	float convergence = 10.f;
	Algebra::Vector4 leftEyeColor = Algebra::Vector4(0.90f, 0.28f, 0.28f, 1.f);
	Algebra::Vector4 rightEyeColor = Algebra::Vector4(0.17f, 0.83f, 0.83f, 1.f);
};
