#include "ShapeInspectorSystem.h"
#include <Algebra.h>
#include <imgui/imgui.h>
#include <UI/Utils.h>

ShapeInspectorSystem::ShapeInspectorSystem(Ref<Scene> scene)
	:m_Scene { scene }
{
	Bind<PositionComponent>(&ShapeInspectorSystem::PositionInspect);
	Bind<RotationComponent>(&ShapeInspectorSystem::RotationInspect);
	Bind<ScaleComponent>(&ShapeInspectorSystem::ScaleInspect);
	Bind<LineGenerationComponent>(&ShapeInspectorSystem::LineInspect);
	Bind<IsParentOfVirtualEntitiesComponent>(&ShapeInspectorSystem::VirtualInspect);
	Bind<TorusGenerationComponent>(&ShapeInspectorSystem::TorusInspect);
}

void ShapeInspectorSystem::Process()
{
	ImGui::Begin("Selected Shapes Properties##Selected Shapes Properties");
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

void ShapeInspectorSystem::PositionInspect(Entity entity)
{
	Algebra::Vector4 tmpPosition = entity.GetComponent<PositionComponent>().position;
	auto& position = entity.GetComponent<PositionComponent>().position;
	if (ImGui::DragFloat3(GUI::GenerateLabel(entity, "Position").c_str(), &tmpPosition.x, 0.1f))
	{
		position = tmpPosition;
	}
}

void ShapeInspectorSystem::ScaleInspect(Entity entity)
{
	auto& scale = entity.GetComponent<ScaleComponent>().scale;
	ImGui::DragFloat3(GUI::GenerateLabel(entity, "Scale").c_str(), &scale.x, 0.1f);
}

void ShapeInspectorSystem::RotationInspect(Entity entity)
{
	auto& rotation = entity.GetComponent<RotationComponent>().rotation;
	ImGui::DragFloat3(GUI::GenerateLabel(entity, "Rotation").c_str(), &rotation.x, 0.1f);
}

void ShapeInspectorSystem::LineInspect(Entity entity)
{
	auto& controlPoints = entity.GetComponent<LineGenerationComponent>().controlPoints;
	ImGui::Text("Control Points: %zu", controlPoints.size());

	for (Entity point : controlPoints)
	{
		if (point.HasComponent<NameComponent>())
			ImGui::Text("Point Name: %s", point.GetComponent<NameComponent>().name.c_str());
	}
}

void ShapeInspectorSystem::VirtualInspect(Entity entity)
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

void ShapeInspectorSystem::TorusInspect(Entity entity)
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

