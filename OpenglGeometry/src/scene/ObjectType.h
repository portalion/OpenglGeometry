#pragma once
#include <array>
#include <optional>
#include <string_view>
#include "Entity.h"

enum class ObjectType
{
	Point,
	Torus,
	Chain,
	BezierC0,
	BezierC2,
	InterpolatedC2,
	BezierSurfaceC0,
	BezierSurfaceC2,
};

inline constexpr std::array AllObjectTypes =
{
	ObjectType::Point,
	ObjectType::Torus,
	ObjectType::Chain,
	ObjectType::BezierC0,
	ObjectType::BezierC2,
	ObjectType::InterpolatedC2,
	ObjectType::BezierSurfaceC0,
	ObjectType::BezierSurfaceC2,
};

inline constexpr std::size_t ObjectTypeCount = AllObjectTypes.size();

struct ObjectTypeComponent
{
	ObjectType type = ObjectType::Point;

	ObjectTypeComponent() = default;
	ObjectTypeComponent(ObjectType type) : type(type) { }
	ObjectTypeComponent(const ObjectTypeComponent& other) = default;
};

inline constexpr const char* ToFormatString(ObjectType type)
{
	switch (type)
	{
	case ObjectType::Point:           return "point";
	case ObjectType::Torus:           return "torus";
	case ObjectType::Chain:           return "chain";
	case ObjectType::BezierC0:        return "bezierC0";
	case ObjectType::BezierC2:        return "bezierC2";
	case ObjectType::InterpolatedC2:  return "interpolatedC2";
	case ObjectType::BezierSurfaceC0: return "bezierSurfaceC0";
	case ObjectType::BezierSurfaceC2: return "bezierSurfaceC2";
	}

	return "point";
}

inline constexpr std::optional<ObjectType> FromFormatString(std::string_view name)
{
	for (ObjectType type : AllObjectTypes)
	{
		if (name == ToFormatString(type))
		{
			return type;
		}
	}

	return std::nullopt;
}

inline constexpr const char* ToDisplayString(ObjectType type)
{
	switch (type)
	{
	case ObjectType::Point:           return "Point";
	case ObjectType::Torus:           return "Torus";
	case ObjectType::Chain:           return "Polyline";
	case ObjectType::BezierC0:        return "Bezier C0";
	case ObjectType::BezierC2:        return "Bezier C2";
	case ObjectType::InterpolatedC2:  return "Interpolated Bezier";
	case ObjectType::BezierSurfaceC0: return "Bezier Surface C0";
	case ObjectType::BezierSurfaceC2: return "Bezier Surface C2";
	}

	return "Object";
}

inline constexpr const char* ToDisplayPluralString(ObjectType type)
{
	switch (type)
	{
	case ObjectType::Point:           return "Points";
	case ObjectType::Torus:           return "Toruses";
	case ObjectType::Chain:           return "Polylines";
	case ObjectType::BezierC0:        return "Bezier C0 curves";
	case ObjectType::BezierC2:        return "Bezier C2 curves";
	case ObjectType::InterpolatedC2:  return "Interpolated Bezier curves";
	case ObjectType::BezierSurfaceC0: return "Bezier C0 surfaces";
	case ObjectType::BezierSurfaceC2: return "Bezier C2 surfaces";
	}

	return "Objects";
}

inline std::optional<ObjectType> GetObjectType(Entity entity)
{
	if (!entity.HasComponent<ObjectTypeComponent>())
	{
		return std::nullopt;
	}

	return entity.GetComponent<ObjectTypeComponent>().type;
}
