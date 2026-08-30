#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
#include "core/Base.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"
#include "interfaces/ICamera.h"
#include "core/ObjectType.h"
#include "core/Log.h"
#include "archetypes/Archetypes.h"

namespace GUI
{
	inline auto GetSceneObjects(Ref<Scene> scene)
	{
		return scene->GetAllEntitiesWith<IdComponent, NameComponent, ObjectTypeComponent>();
	}

	inline void SelectAll(Ref<Scene> scene)
	{
		for (Entity entity : GetSceneObjects(scene))
		{
			if (!entity.HasComponent<IsSelectedTag>())
			{
				entity.AddTag<IsSelectedTag>();
			}
		}
	}

	inline void DeselectAll(Ref<Scene> scene)
	{
		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag>())
		{
			entity.RemoveTag<IsSelectedTag>();
		}
	}

	inline bool IsSurfaceControlPoint(Entity entity)
	{
		if (!entity.IsValid() || !entity.HasComponent<NotificationComponent>())
		{
			return false;
		}

		for (Entity notified : entity.GetComponent<NotificationComponent>().entitiesToNotify)
		{
			if (notified.IsValid() && notified.HasComponent<BezierPatchGenerationComponent>())
			{
				return true;
			}
		}

		return false;
	}

	inline void DeleteSelected(Ref<Scene> scene)
	{
		int count = 0;
		int locked = 0;
		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag>())
		{
			if (IsSurfaceControlPoint(entity))
			{
				locked++;
				continue;
			}

			entity.AddTag<ToBeDestroyedTag>();
			count++;
		}

		if (count > 0)
		{
			Logger::Info("Deleted {} selected object(s)", count);
		}
		if (locked > 0)
		{
			Logger::Warning("Kept {} surface control point(s) - a surface's points cannot be deleted", locked);
		}
	}

	inline bool AnythingSelected(Ref<Scene> scene)
	{
		return !scene->GetAllEntitiesWith<IsSelectedTag>().empty();
	}

	inline Entity SingleSelected(Ref<Scene> scene)
	{
		Entity result;

		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, NameComponent>())
		{
			if (result.IsValid())
			{
				return Entity{};
			}

			result = entity;
		}

		return result;
	}

	inline bool CanRename(Ref<Scene> scene)
	{
		return SingleSelected(scene).IsValid();
	}

	inline void FocusSelected(Ref<Scene> scene)
	{
		std::vector<Algebra::Vector4> points;

		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, PositionComponent>())
		{
			points.push_back(entity.GetComponent<PositionComponent>().position);
		}

		if (points.empty())
		{
			points.push_back(Archetypes::GetCursorPosition(scene.get()));
		}

		Algebra::Vector4 center = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
		for (const auto& point : points)
		{
			center = center + Algebra::Vector4(point.x, point.y, point.z, 0.f);
		}
		center = center / static_cast<float>(points.size());

		float radius = 0.f;
		for (const auto& point : points)
		{
			radius = std::max(radius, (Algebra::Vector4(point.x, point.y, point.z, 0.f) - center).Length());
		}

		center.w = 1.f;

		for (Entity entity : scene->GetAllEntitiesWith<CameraComponent>())
		{
			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			if (!cameraComponent.active || !cameraComponent.cameraHandling)
			{
				continue;
			}

			cameraComponent.cameraHandling->Focus(center, radius);
		}
	}

	inline void SelectOnly(Ref<Scene> scene, Entity entity)
	{
		DeselectAll(scene);
		entity.AddTag<IsSelectedTag>();
	}

	inline std::vector<Entity> GetSelectedControlPoints(Ref<Scene> scene)
	{
		auto view = scene->GetAllEntitiesWith<IsSelectedTag, NotificationComponent>();
		return std::vector<Entity>(view.begin(), view.end());
	}

	inline Entity CreateShape(Ref<Scene> scene, ObjectType type)
	{
		Scene* raw = scene.get();
		const auto cursor = Archetypes::GetCursorPosition(raw);

		Entity created;

		switch (type)
		{
		case ObjectType::Point:
			created = Archetypes::CreatePoint(raw, cursor);
			break;
		case ObjectType::Torus:
			created = Archetypes::CreateTorus(raw, cursor);
			break;
		case ObjectType::Chain:
		{
			auto points = GetSelectedControlPoints(scene);
			created = Archetypes::CreatePolyline(raw, points.begin(), points.end());
			break;
		}
		case ObjectType::BezierC0:
		{
			auto points = GetSelectedControlPoints(scene);
			created = Archetypes::CreateBezierC0(raw, points.begin(), points.end());
			break;
		}
		case ObjectType::BezierC2:
		{
			auto points = GetSelectedControlPoints(scene);
			created = Archetypes::CreateBezierC2(raw, points.begin(), points.end());
			break;
		}
		case ObjectType::InterpolatedC2:
		{
			auto points = GetSelectedControlPoints(scene);
			created = Archetypes::CreateInterpolatedBezier(raw, points.begin(), points.end());
			break;
		}
		default:
			return Entity{};
		}

		if (created.IsValid())
		{
			Logger::Info("Created {}", ToDisplayString(type));
		}
		else
		{
			Logger::Warning("Could not create {}", ToDisplayString(type));
		}

		return created;
	}

	inline void SyncSelectionCentreVisibility(Ref<Scene> scene, bool visible)
	{
		for (Entity marker : scene->GetAllEntitiesWith<SelectionCentreTag>())
		{
			const bool hidden = marker.HasComponent<SelectionCentreHiddenTag>();
			if (visible && hidden)
			{
				marker.RemoveTag<SelectionCentreHiddenTag>();
			}
			else if (!visible && !hidden)
			{
				marker.AddTag<SelectionCentreHiddenTag>();
			}
		}
	}

	enum class TransformSpace
	{
		Local,
		Cursor,
		Centre,
	};

	struct SelectionTransform
	{
		Algebra::Vector4 translation = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
		Algebra::Vector4 rotationDegrees = Algebra::Vector4(0.f, 0.f, 0.f, 0.f);
		Algebra::Vector4 scale = Algebra::Vector4(1.f, 1.f, 1.f, 0.f);
	};

	inline std::vector<Entity> GetSelectedTransformables(Ref<Scene> scene)
	{
		auto view = scene->GetAllEntitiesWith<IsSelectedTag, PositionComponent>();
		return std::vector<Entity>(view.begin(), view.end());
	}

	inline Algebra::Vector4 SelectionCentre(Ref<Scene> scene)
	{
		Algebra::Vector4 sum(0.f, 0.f, 0.f, 0.f);
		int count = 0;

		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, PositionComponent>())
		{
			const Algebra::Vector4 position = entity.GetComponent<PositionComponent>().position;
			sum = sum + Algebra::Vector4(position.x, position.y, position.z, 0.f);
			count++;
		}

		if (count == 0)
		{
			return Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
		}

		Algebra::Vector4 centre = sum / static_cast<float>(count);
		centre.w = 1.f;
		return centre;
	}

	inline Algebra::Vector4 QuaternionToEulerDegrees(const Algebra::Quaternion& q)
	{
		const float sinRoll = 2.f * (q.w * q.x + q.y * q.z);
		const float cosRoll = 1.f - 2.f * (q.x * q.x + q.y * q.y);
		const float roll = std::atan2(sinRoll, cosRoll);

		float sinPitch = 2.f * (q.w * q.y - q.z * q.x);
		sinPitch = std::clamp(sinPitch, -1.f, 1.f);
		const float pitch = std::asin(sinPitch);

		const float sinYaw = 2.f * (q.w * q.z + q.x * q.y);
		const float cosYaw = 1.f - 2.f * (q.y * q.y + q.z * q.z);
		const float yaw = std::atan2(sinYaw, cosYaw);

		return Algebra::Vector4(
			Algebra::RadiansToDegree(roll),
			Algebra::RadiansToDegree(pitch),
			Algebra::RadiansToDegree(yaw),
			0.f);
	}

	inline Algebra::Quaternion EulerDegreesToQuaternion(const Algebra::Vector4& degrees)
	{
		const Algebra::Quaternion qx = Algebra::Quaternion::CreateFromAxisAngle(
			Algebra::Vector4(1.f, 0.f, 0.f, 0.f), Algebra::DegreeToRadians(degrees.x));
		const Algebra::Quaternion qy = Algebra::Quaternion::CreateFromAxisAngle(
			Algebra::Vector4(0.f, 1.f, 0.f, 0.f), Algebra::DegreeToRadians(degrees.y));
		const Algebra::Quaternion qz = Algebra::Quaternion::CreateFromAxisAngle(
			Algebra::Vector4(0.f, 0.f, 1.f, 0.f), Algebra::DegreeToRadians(degrees.z));

		return (qz * qy * qx).Normalize();
	}

	inline void ApplySelectionTransform(Ref<Scene> scene, TransformSpace space, const SelectionTransform& t)
	{
		const std::vector<Entity> selected = GetSelectedTransformables(scene);
		if (selected.empty())
		{
			return;
		}

		const Algebra::Quaternion delta = EulerDegreesToQuaternion(t.rotationDegrees);
		const Algebra::Vector4 scale = t.scale;

		Algebra::Vector4 pivot(0.f, 0.f, 0.f, 1.f);
		if (space == TransformSpace::Cursor)
		{
			pivot = Archetypes::GetCursorPosition(scene.get());
		}
		else if (space == TransformSpace::Centre)
		{
			pivot = SelectionCentre(scene);
		}

		for (Entity entity : selected)
		{
			const Algebra::Vector4 position = entity.GetComponent<PositionComponent>().position;

			const bool hasRotation = entity.HasComponent<RotationComponent>();
			const bool hasScale = entity.HasComponent<ScaleComponent>();
			const Algebra::Quaternion rotation = hasRotation
				? entity.GetComponent<RotationComponent>().rotation
				: Algebra::Quaternion::Identity();

			Algebra::Vector4 newPosition = position;

			if (space == TransformSpace::Local)
			{
				newPosition = position + rotation.Rotate(t.translation);
			}
			else
			{
				Algebra::Vector4 relative(position.x - pivot.x, position.y - pivot.y, position.z - pivot.z, 0.f);
				relative = relative.Scale(scale);
				relative = delta.Rotate(relative);
				newPosition = Algebra::Vector4(pivot.x + relative.x, pivot.y + relative.y, pivot.z + relative.z, 1.f)
					+ t.translation;
			}

			entity.GetComponent<PositionComponent>().position = newPosition;

			if (hasRotation)
			{
				const Algebra::Quaternion newRotation = space == TransformSpace::Local
					? rotation * delta
					: delta * rotation;
				entity.GetComponent<RotationComponent>().rotation = newRotation.Normalize();
			}

			if (hasScale)
			{
				entity.GetComponent<ScaleComponent>().scale =
					entity.GetComponent<ScaleComponent>().scale.Scale(scale);
			}
		}
	}
}
