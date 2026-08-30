#include "SceneSerialization.h"

#include <fstream>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <nlohmann/json.hpp>

#include "Algebra.h"
#include "archetypes/Archetypes.h"
#include "core/Log.h"
#include "core/ObjectType.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/ObjectType.h"
#include "scene/Scene.h"
#include "scene/Tags.h"

using json = nlohmann::json;

namespace
{
	struct Uint2 { uint32_t u = 0; uint32_t v = 0; };

	struct PointDto
	{
		uint32_t id = 0;
		std::optional<std::string> name;
		Algebra::Vector4 position;
	};

	struct GeometryDto
	{
		ObjectType type = ObjectType::Point;
		uint32_t id = 0;
		std::optional<std::string> name;

		Algebra::Vector4 position;
		Algebra::Vector4 scale = Algebra::Vector4(1.f, 1.f, 1.f, 0.f);
		Algebra::Quaternion rotation;
		Uint2 samples;
		float smallRadius = 0.f;
		float largeRadius = 0.f;

		std::vector<uint32_t> controlPoints;
		Uint2 size;
	};

	struct SceneDto
	{
		std::vector<PointDto> points;
		std::vector<GeometryDto> geometry;
		int skipped = 0;
	};

#pragma region JSON helpers

	json Float3Json(const Algebra::Vector4& v)
	{
		return json::object({ { "x", v.x }, { "y", v.y }, { "z", v.z } });
	}

	json QuaternionJson(const Algebra::Quaternion& q)
	{
		return json::object({ { "x", q.x }, { "y", q.y }, { "z", q.z }, { "w", q.w } });
	}

	json Uint2Json(uint32_t u, uint32_t v)
	{
		return json::object({ { "u", u }, { "v", v } });
	}

	Algebra::Vector4 ReadFloat3(const json& j, float w = 0.f)
	{
		return Algebra::Vector4(
			j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>(), w);
	}

	json PointRef(uint32_t id)
	{
		return json::object({ { "id", id } });
	}

#pragma endregion

#pragma region Export

	bool IsCurveType(ObjectType type)
	{
		return type == ObjectType::Chain || type == ObjectType::BezierC0
			|| type == ObjectType::BezierC2 || type == ObjectType::InterpolatedC2;
	}

	std::optional<uint32_t> IdOf(Entity entity)
	{
		if (!entity.IsValid() || !entity.HasComponent<IdComponent>())
		{
			return std::nullopt;
		}
		return entity.GetComponent<IdComponent>().id;
	}

	std::vector<std::vector<uint32_t>> ReconstructSurfaceGrid(
		const BezierSurfaceGenerationComponent& surface, unsigned int stride, Uint2& outSize, bool& outOk)
	{
		outOk = false;
		const auto& patches = surface.bezierPatches;
		if (patches.empty() || patches[0].empty())
		{
			return {};
		}

		const unsigned int numXPatches = static_cast<unsigned int>(patches.size());
		const unsigned int numYPatches = static_cast<unsigned int>(patches[0].size());
		const unsigned int overlap = 4u - stride;
		const unsigned int sizeU = stride * numXPatches + overlap;
		const unsigned int sizeV = stride * numYPatches + overlap;

		std::vector<std::vector<uint32_t>> grid(sizeU, std::vector<uint32_t>(sizeV, 0));
		std::vector<std::vector<bool>> filled(sizeU, std::vector<bool>(sizeV, false));

		for (unsigned int i = 0; i < numXPatches; i++)
		{
			for (unsigned int j = 0; j < numYPatches; j++)
			{
				Entity patch = patches[i][j];
				if (!patch.IsValid() || !patch.HasComponent<BezierPatchGenerationComponent>())
				{
					return {};
				}
				const auto& patchPoints = patch.GetComponent<BezierPatchGenerationComponent>().controlPoints;

				for (unsigned int x = 0; x < 4; x++)
				{
					for (unsigned int y = 0; y < 4; y++)
					{
						const std::optional<uint32_t> id = IdOf(patchPoints[x][y]);
						if (!id)
						{
							return {};
						}
						grid[i * stride + x][j * stride + y] = *id;
						filled[i * stride + x][j * stride + y] = true;
					}
				}
			}
		}

		for (unsigned int u = 0; u < sizeU; u++)
		{
			for (unsigned int v = 0; v < sizeV; v++)
			{
				if (!filled[u][v])
				{
					return {};
				}
			}
		}

		outSize = { sizeU, sizeV };
		outOk = true;
		return grid;
	}

	json SerializeSurface(Entity entity, ObjectType type)
	{
		const auto& surface = entity.GetComponent<BezierSurfaceGenerationComponent>();
		const unsigned int stride = type == ObjectType::BezierSurfaceC2 ? 1u : 3u;

		Uint2 size;
		bool ok = false;
		const auto grid = ReconstructSurfaceGrid(surface, stride, size, ok);
		if (!ok)
		{
			Logger::Warning("Serialization: skipping surface id {} (incomplete control net)",
				entity.GetComponent<IdComponent>().id);
			return json();
		}

		json result;
		result["objectType"] = ToFormatString(type);
		result["id"] = entity.GetComponent<IdComponent>().id;
		result["name"] = entity.GetComponent<NameComponent>().name;
		result["size"] = Uint2Json(size.u, size.v);
		result["samples"] = Uint2Json(
			static_cast<uint32_t>(surface.samplesU), static_cast<uint32_t>(surface.samplesV));

		result["controlPoints"] = json::array();
		for (uint32_t v = 0; v < size.v; v++)
		{
			for (uint32_t u = 0; u < size.u; u++)
			{
				result["controlPoints"].push_back(PointRef(grid[u][v]));
			}
		}
		return result;
	}

	json SerializeCurve(Entity entity, ObjectType type)
	{
		json result;
		result["objectType"] = ToFormatString(type);
		result["id"] = entity.GetComponent<IdComponent>().id;
		result["name"] = entity.GetComponent<NameComponent>().name;
		result["controlPoints"] = json::array();

		auto& controlPoints = entity.GetComponent<LineGenerationComponent>().controlPoints;
		for (Entity point : controlPoints)
		{
			if (const std::optional<uint32_t> id = IdOf(point))
			{
				result["controlPoints"].push_back(PointRef(*id));
			}
		}
		return result;
	}

	json SerializeTorus(Entity entity)
	{
		const auto& torus = entity.GetComponent<TorusGenerationComponent>();

		const Algebra::Vector4 position = entity.GetComponent<PositionComponent>().position;
		const Algebra::Quaternion rotation = entity.HasComponent<RotationComponent>()
			? entity.GetComponent<RotationComponent>().rotation
			: Algebra::Quaternion::Identity();
		const Algebra::Vector4 scale = entity.HasComponent<ScaleComponent>()
			? entity.GetComponent<ScaleComponent>().scale
			: Algebra::Vector4(1.f, 1.f, 1.f, 0.f);

		json result;
		result["objectType"] = "torus";
		result["id"] = entity.GetComponent<IdComponent>().id;
		result["name"] = entity.GetComponent<NameComponent>().name;
		result["position"] = Float3Json(position);
		result["rotation"] = QuaternionJson(rotation);
		result["scale"] = Float3Json(scale);
		result["samples"] = Uint2Json(torus.radialSegments, torus.tubularSegments);
		result["smallRadius"] = torus.tubeRadius;
		result["largeRadius"] = torus.radius;
		return result;
	}

	json SerializeScene(Scene& scene)
	{
		json result;
		result["points"] = json::array();
		result["geometry"] = json::array();

		for (Entity entity : scene.GetAllEntitiesWith<IdComponent, ObjectTypeComponent>())
		{
			const ObjectType type = entity.GetComponent<ObjectTypeComponent>().type;
			const uint32_t id = entity.GetComponent<IdComponent>().id;
			const std::string name = entity.HasComponent<NameComponent>()
				? entity.GetComponent<NameComponent>().name : std::string();

			if (type == ObjectType::Point)
			{
				json point;
				point["id"] = id;
				point["name"] = name;
				point["position"] = Float3Json(entity.GetComponent<PositionComponent>().position);
				result["points"].push_back(point);
				continue;
			}

			json geometry;
			if (type == ObjectType::Torus)
			{
				geometry = SerializeTorus(entity);
			}
			else if (IsCurveType(type))
			{
				geometry = SerializeCurve(entity, type);
			}
			else if (type == ObjectType::BezierSurfaceC0 || type == ObjectType::BezierSurfaceC2)
			{
				geometry = SerializeSurface(entity, type);
			}

			if (!geometry.is_null())
			{
				result["geometry"].push_back(geometry);
			}
		}

		return result;
	}

#pragma endregion

#pragma region Parse

	std::optional<std::string> ReadName(const json& j)
	{
		if (j.contains("name") && j.at("name").is_string())
		{
			return j.at("name").get<std::string>();
		}
		return std::nullopt;
	}

	Uint2 ReadUint2(const json& j)
	{
		return { j.at("u").get<uint32_t>(), j.at("v").get<uint32_t>() };
	}

	std::vector<uint32_t> ReadControlPoints(const json& j)
	{
		std::vector<uint32_t> result;
		for (const json& ref : j.at("controlPoints"))
		{
			result.push_back(ref.at("id").get<uint32_t>());
		}
		return result;
	}

	SceneDto ParseSceneDto(const json& root)
	{
		SceneDto dto;

		if (root.contains("points"))
		{
			for (const json& j : root.at("points"))
			{
				PointDto point;
				point.id = j.at("id").get<uint32_t>();
				point.name = ReadName(j);
				point.position = ReadFloat3(j.at("position"), 1.f);
				dto.points.push_back(point);
			}
		}

		if (root.contains("geometry"))
		{
			for (const json& j : root.at("geometry"))
			{
				const std::string typeStr = j.at("objectType").get<std::string>();
				const std::optional<ObjectType> type = FromFormatString(typeStr);
				if (!type || *type == ObjectType::Point)
				{
					Logger::Warning("Load: skipping unknown objectType \"{}\"", typeStr);
					dto.skipped++;
					continue;
				}

				GeometryDto geometry;
				geometry.type = *type;
				geometry.id = j.at("id").get<uint32_t>();
				geometry.name = ReadName(j);

				if (*type == ObjectType::Torus)
				{
					geometry.position = ReadFloat3(j.at("position"), 1.f);
					const json& r = j.at("rotation");
					geometry.rotation = Algebra::Quaternion(
						r.at("w").get<float>(), r.at("x").get<float>(),
						r.at("y").get<float>(), r.at("z").get<float>());
					geometry.scale = ReadFloat3(j.at("scale"), 0.f);
					geometry.samples = ReadUint2(j.at("samples"));
					geometry.smallRadius = j.at("smallRadius").get<float>();
					geometry.largeRadius = j.at("largeRadius").get<float>();
				}
				else
				{
					geometry.controlPoints = ReadControlPoints(j);
					if (*type == ObjectType::BezierSurfaceC0 || *type == ObjectType::BezierSurfaceC2)
					{
						geometry.size = ReadUint2(j.at("size"));
						geometry.samples = ReadUint2(j.at("samples"));
					}
				}

				dto.geometry.push_back(geometry);
			}
		}

		return dto;
	}

#pragma endregion

#pragma region Validate

	bool AxisConsistentForC0(uint32_t size)
	{
		return size >= 4 && (size - 1) % 3 == 0;
	}

	std::optional<std::string> ValidateSceneDto(const SceneDto& dto)
	{
		std::unordered_set<uint32_t> pointIds;
		std::unordered_set<uint32_t> allIds;

		for (const PointDto& point : dto.points)
		{
			if (!allIds.insert(point.id).second)
			{
				return "duplicate id " + std::to_string(point.id);
			}
			pointIds.insert(point.id);
		}

		for (const GeometryDto& geometry : dto.geometry)
		{
			if (!allIds.insert(geometry.id).second)
			{
				return "duplicate id " + std::to_string(geometry.id);
			}

			if (geometry.type == ObjectType::Torus)
			{
				continue;
			}

			for (uint32_t ref : geometry.controlPoints)
			{
				if (!pointIds.contains(ref))
				{
					return "object " + std::to_string(geometry.id)
						+ " references missing point " + std::to_string(ref);
				}
			}

			if (geometry.type == ObjectType::BezierSurfaceC0 || geometry.type == ObjectType::BezierSurfaceC2)
			{
				const uint32_t expected = geometry.size.u * geometry.size.v;
				if (geometry.controlPoints.size() != expected)
				{
					return "surface " + std::to_string(geometry.id) + " has "
						+ std::to_string(geometry.controlPoints.size())
						+ " control points, size says " + std::to_string(expected);
				}

				const bool c2 = geometry.type == ObjectType::BezierSurfaceC2;
				const bool uOk = c2 ? geometry.size.u >= 4 : AxisConsistentForC0(geometry.size.u);
				const bool vOk = c2 ? geometry.size.v >= 4 : AxisConsistentForC0(geometry.size.v);
				if (!uOk || !vOk)
				{
					return "surface " + std::to_string(geometry.id) + " size "
						+ std::to_string(geometry.size.u) + "x" + std::to_string(geometry.size.v)
						+ " is not a whole number of patches";
				}
			}
			else if (geometry.controlPoints.empty())
			{
				return "curve " + std::to_string(geometry.id) + " has no control points";
			}
		}

		return std::nullopt;
	}

#pragma endregion

#pragma region Apply

	void SetName(Entity entity, const std::optional<std::string>& name)
	{
		if (name && entity.IsValid() && entity.HasComponent<NameComponent>())
		{
			entity.GetComponent<NameComponent>().name = *name;
		}
	}

	Entity CreateTorusFromDto(Scene& scene, const GeometryDto& dto)
	{
		Entity entity = scene.CreateEntity();
		Archetypes::AddShapeToEntity(entity, ObjectType::Torus);
		Archetypes::AddTorusToEntity(entity, dto.position);

		auto& torus = entity.GetComponent<TorusGenerationComponent>();
		torus.radius = dto.largeRadius;
		torus.tubeRadius = dto.smallRadius;
		torus.radialSegments = dto.samples.u;
		torus.tubularSegments = dto.samples.v;

		entity.GetComponent<RotationComponent>().rotation = dto.rotation.Normalize();
		entity.GetComponent<ScaleComponent>().scale =
			Algebra::Vector4(dto.scale.x, dto.scale.y, dto.scale.z, 0.f);

		entity.AddTag<IsDirtyTag>();
		return entity;
	}

	Entity CreateCurveFromDto(Scene& scene, const GeometryDto& dto,
		const std::unordered_map<uint32_t, Entity>& pointsById)
	{
		std::vector<Entity> points;
		points.reserve(dto.controlPoints.size());
		for (uint32_t ref : dto.controlPoints)
		{
			points.push_back(pointsById.at(ref));
		}

		Scene* raw = &scene;
		switch (dto.type)
		{
		case ObjectType::Chain:
			return Archetypes::CreatePolyline(raw, points.begin(), points.end());
		case ObjectType::BezierC0:
			return Archetypes::CreateBezierC0(raw, points.begin(), points.end());
		case ObjectType::BezierC2:
			return Archetypes::CreateBezierC2(raw, points.begin(), points.end());
		case ObjectType::InterpolatedC2:
			return Archetypes::CreateInterpolatedBezier(raw, points.begin(), points.end());
		default:
			return Entity{};
		}
	}

	Entity CreateSurfaceFromDto(Scene& scene, const GeometryDto& dto,
		const std::unordered_map<uint32_t, Entity>& pointsById)
	{
		const uint32_t sizeU = dto.size.u;
		const uint32_t sizeV = dto.size.v;

		std::vector<std::vector<Entity>> grid(sizeU, std::vector<Entity>(sizeV));
		for (uint32_t v = 0; v < sizeV; v++)
		{
			for (uint32_t u = 0; u < sizeU; u++)
			{
				grid[u][v] = pointsById.at(dto.controlPoints[v * sizeU + u]);
			}
		}

		const bool c2 = dto.type == ObjectType::BezierSurfaceC2;
		return Archetypes::CreateBezierSurfaceFromControlGrid(
			&scene, grid, static_cast<int>(dto.samples.u), static_cast<int>(dto.samples.v), c2);
	}

	void ApplySceneDto(Scene& scene, const SceneDto& dto)
	{
		scene.Clear();

		std::unordered_map<uint32_t, Entity> pointsById;
		for (const PointDto& point : dto.points)
		{
			Entity entity = Archetypes::CreatePoint(&scene,
				Algebra::Vector4(point.position.x, point.position.y, point.position.z, 1.f));
			SetName(entity, point.name);
			pointsById[point.id] = entity;
		}

		for (const GeometryDto& geometry : dto.geometry)
		{
			Entity entity;
			if (geometry.type == ObjectType::Torus)
			{
				entity = CreateTorusFromDto(scene, geometry);
			}
			else if (geometry.type == ObjectType::BezierSurfaceC0 || geometry.type == ObjectType::BezierSurfaceC2)
			{
				entity = CreateSurfaceFromDto(scene, geometry, pointsById);
			}
			else
			{
				entity = CreateCurveFromDto(scene, geometry, pointsById);
			}

			SetName(entity, geometry.name);
		}
	}

#pragma endregion
}

namespace Serialization
{
	Result SaveScene(Scene& scene, const std::string& path)
	{
		json root;
		try
		{
			root = SerializeScene(scene);
		}
		catch (const std::exception& e)
		{
			const std::string message = std::string("could not serialize the scene: ") + e.what();
			Logger::Error("Save failed: {}", message);
			return { false, message };
		}

		std::ofstream file(path, std::ios::trunc);
		if (!file)
		{
			const std::string message = "could not open " + path + " for writing";
			Logger::Error("Save failed: {}", message);
			return { false, message };
		}

		file << root.dump(2) << '\n';
		if (!file)
		{
			const std::string message = "write to " + path + " failed";
			Logger::Error("Save failed: {}", message);
			return { false, message };
		}

		const std::size_t points = root.value("points", json::array()).size();
		const std::size_t geometry = root.value("geometry", json::array()).size();
		const std::string message = "saved " + path + " ("
			+ std::to_string(points) + " points, " + std::to_string(geometry) + " objects)";
		Logger::Info("{}", message);
		return { true, message };
	}

	Result LoadScene(Scene& scene, const std::string& path)
	{
		std::ifstream file(path);
		if (!file)
		{
			const std::string message = "could not open " + path;
			Logger::Error("Load failed: {}", message);
			return { false, message };
		}

		json root;
		SceneDto dto;
		try
		{
			file >> root;
			dto = ParseSceneDto(root);
		}
		catch (const std::exception& e)
		{
			const std::string message = std::string(path) + ": " + e.what();
			Logger::Error("Load failed: {}", message);
			return { false, message };
		}

		if (const std::optional<std::string> error = ValidateSceneDto(dto))
		{
			const std::string message = std::string(path) + ": " + *error;
			Logger::Error("Load failed: {}", message);
			return { false, message };
		}

		try
		{
			ApplySceneDto(scene, dto);
		}
		catch (const std::exception& e)
		{
			const std::string message = std::string("load aborted while building the scene: ") + e.what();
			Logger::Error("{}", message);
			return { false, message };
		}

		std::ostringstream message;
		message << "loaded " << path << ": " << dto.points.size() << " points, "
			<< dto.geometry.size() << " objects";
		if (dto.skipped > 0)
		{
			message << " (" << dto.skipped << " skipped)";
		}
		Logger::Info("{}", message.str());
		return { true, message.str() };
	}
}
