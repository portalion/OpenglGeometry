#include "ShapeInspectorRegistry.h"
#include <Algebra.h>
#include <imgui/imgui.h>
#include <UI/Utils.h>

ShapeInspectorRegistry::ShapeInspectorRegistry(Ref<Scene> scene)
	:m_Scene { scene }
{
	Bind<PositionComponent>(&ShapeInspectorRegistry::PositionInspect);
	Bind<RotationComponent>(&ShapeInspectorRegistry::RotationInspect);
	Bind<ScaleComponent>(&ShapeInspectorRegistry::ScaleInspect);
	Bind<LineGenerationComponent>(&ShapeInspectorRegistry::LineInspect);
	Bind<IsParentOfVirtualEntitiesComponent>(&ShapeInspectorRegistry::VirtualInspect);
	Bind<TorusGenerationComponent>(&ShapeInspectorRegistry::TorusInspect);
}

void ShapeInspectorRegistry::Display()
{
	ImGui::Begin(GUI::InspectorWindow);
	auto selectedShapes = m_Scene->GetAllEntitiesWith<IsSelectedTag>();

	if (selectedShapes.empty())
	{
		ImGui::Text("No shapes selected.");
	}

	for (Entity entity : selectedShapes)
	{
		ImGui::Text("Properties of %s", entity.GetComponent<NameComponent>().name.c_str());
		this->PerformFunctions(entity);
	}

	ImGui::End();
}

void ShapeInspectorRegistry::PositionInspect(Entity entity)
{
	Algebra::Vector4 tmpPosition = entity.GetComponent<PositionComponent>().position;
	auto& position = entity.GetComponent<PositionComponent>().position;
	if (ImGui::DragFloat3(GUI::GenerateLabel(entity, "Position").c_str(), &tmpPosition.x, 0.1f))
	{
		position = tmpPosition;
	}
}

void ShapeInspectorRegistry::ScaleInspect(Entity entity)
{
	auto& scale = entity.GetComponent<ScaleComponent>().scale;
	ImGui::DragFloat3(GUI::GenerateLabel(entity, "Scale").c_str(), &scale.x, 0.1f);
}

void ShapeInspectorRegistry::RotationInspect(Entity entity)
{
	auto& rotation = entity.GetComponent<RotationComponent>().rotation;
	ImGui::DragFloat3(GUI::GenerateLabel(entity, "Rotation").c_str(), &rotation.x, 0.1f);
}

void ShapeInspectorRegistry::LineInspect(Entity entity)
{
	auto& controlPoints = entity.GetComponent<LineGenerationComponent>().controlPoints;
	ImGui::Text("Control Points: %zu", controlPoints.size());

	for (Entity point : controlPoints)
	{
		if (point.HasComponent<NameComponent>())
			ImGui::Text("Point Name: %s", point.GetComponent<NameComponent>().name.c_str());
	}
}

void ShapeInspectorRegistry::VirtualInspect(Entity entity)
{
	auto virtualEntities = entity.GetComponent<IsParentOfVirtualEntitiesComponent>().virtualEntities;
	for (Entity virtualEntity : virtualEntities)
	{
		if (!virtualEntity.IsValid()) continue;

		bool visible = !virtualEntity.HasComponent<IsInvisibleTag>();

		if (ImGui::Checkbox(GUI::GenerateLabel(entity, "virtual visible").c_str(), &visible))
		{
			if (visible)
			{
				virtualEntity.RemoveTag<IsInvisibleTag>();
			}
			else
			{
				virtualEntity.AddTag<IsInvisibleTag>();
			}
		}
	}
}

void ShapeInspectorRegistry::TorusInspect(Entity entity)
{
	auto& torusComponent = entity.GetComponent<TorusGenerationComponent>();

	if (ImGui::DragFloat(GUI::GenerateLabel(entity, "Radius").c_str(), &torusComponent.radius, 0.1f))
	{
		entity.AddTag<IsDirtyTag>();
	}
	if (ImGui::DragFloat(GUI::GenerateLabel(entity, "Tube Radius").c_str(), &torusComponent.tubeRadius, 0.1f))
	{
		entity.AddTag<IsDirtyTag>();
	}
	if (GUI::DragUInt(GUI::GenerateLabel(entity, "Radial Segments").c_str(), &torusComponent.radialSegments, 1.0f, 3, 64))
	{
		entity.AddTag<IsDirtyTag>();
	}
	if (GUI::DragUInt(GUI::GenerateLabel(entity, "Tubular Segments").c_str(), &torusComponent.tubularSegments, 1.0f, 3, 64))
	{
		entity.AddTag<IsDirtyTag>();
	}
}

