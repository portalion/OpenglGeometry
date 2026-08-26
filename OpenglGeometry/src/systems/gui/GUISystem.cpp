#include "GUISystem.h"
#include "UI/GUI.h"
#include <archetypes/SimpleArchetypeCreation.h>
#include <managers/StaticMeshManager.h>
#include <ui/SceneActions.h>
#include <ui/layouts/DefaultLayout.h>

namespace
{
	GUI::TransformSpace ToTransformSpace(PivotMode pivot)
	{
		switch (pivot)
		{
		case PivotMode::Origin: return GUI::TransformSpace::Local;
		case PivotMode::Cursor: return GUI::TransformSpace::Cursor;
		case PivotMode::Centre:
		default:                return GUI::TransformSpace::Centre;
		}
	}
}

GUISystem::GUISystem(Ref<Scene> scene, Viewport& viewport)
	:m_Scene(scene), m_Viewport(viewport), m_ShapeList{ scene }
{
	m_Callbacks.resetLayout = [this]() { GUI::Layout::Default(this->m_Dockspace); };
	m_Callbacks.renameSelected = [this]() { this->m_ShapeList.RequestRename(); };

	m_InspectorCallbacks.applySelectionTransform =
		[this](PivotMode pivot, Algebra::Vector4 moveBy, Algebra::Vector4 rotateBy, Algebra::Vector4 scaleBy)
		{
			GUI::SelectionTransform transform;
			transform.translation = moveBy;
			transform.rotationDegrees = rotateBy;
			transform.scale = scaleBy;
			GUI::ApplySelectionTransform(this->m_Scene, ToTransformSpace(pivot), transform);
		};
}

void GUISystem::SyncInspectorState()
{
	m_UiState.objects.clear();
	for (Entity entity : GUI::GetSceneObjects(m_Scene))
	{
		ObjectRow row;
		row.id = entity.GetComponent<IdComponent>().id;
		row.name = entity.GetComponent<NameComponent>().name;
		row.type = entity.GetComponent<ObjectTypeComponent>().type;
		row.selected = entity.HasComponent<IsSelectedTag>();
		m_UiState.objects.push_back(row);
	}

	m_UiState.cursor.selectionCentre = GUI::SelectionCentre(m_Scene);

	m_UiState.transform.reset();
	m_UiState.torus.reset();
	m_UiState.curve.reset();
	m_UiState.surface.reset();
	m_InspectedEntity = Entity{};

	Entity single;
	int count = 0;
	for (Entity entity : m_Scene->GetAllEntitiesWith<IsSelectedTag>())
	{
		single = entity;
		if (++count > 1)
		{
			break;
		}
	}

	if (count != 1)
	{
		return;
	}

	m_InspectedEntity = single;

	if (single.HasComponent<NameComponent>())
	{
		m_NameSnapshot = single.GetComponent<NameComponent>().name;
	}

	if (single.HasComponent<PositionComponent>())
	{
		TransformValues values;
		values.position = single.GetComponent<PositionComponent>().position;

		if (single.HasComponent<RotationComponent>())
		{
			values.rotationEuler = GUI::QuaternionToEulerDegrees(single.GetComponent<RotationComponent>().rotation);
		}

		values.scale = single.HasComponent<ScaleComponent>()
			? single.GetComponent<ScaleComponent>().scale
			: Algebra::Vector4(1.f, 1.f, 1.f, 0.f);

		m_UiState.transform = values;
		m_TransformSnapshot = values;
	}

	if (single.HasComponent<TorusGenerationComponent>())
	{
		const auto& torus = single.GetComponent<TorusGenerationComponent>();
		TorusValues values;
		values.largeRadius = torus.radius;
		values.tubeRadius = torus.tubeRadius;
		values.samplesU = torus.radialSegments;
		values.samplesV = torus.tubularSegments;

		m_UiState.torus = values;
		m_TorusSnapshot = values;
	}
}

void GUISystem::WriteBackInspectorState()
{
	if (!m_InspectedEntity.IsValid())
	{
		return;
	}

	Entity entity = m_InspectedEntity;

	if (entity.HasComponent<NameComponent>() && entity.HasComponent<IdComponent>())
	{
		const auto id = entity.GetComponent<IdComponent>().id;
		for (const ObjectRow& row : m_UiState.objects)
		{
			if (row.id == id && row.name != m_NameSnapshot)
			{
				entity.GetComponent<NameComponent>().name = row.name;
			}
		}
	}

	if (m_UiState.transform && entity.HasComponent<PositionComponent>())
	{
		const TransformValues& values = *m_UiState.transform;

		if (!(values.position == m_TransformSnapshot.position))
		{
			entity.GetComponent<PositionComponent>().position =
				Algebra::Vector4(values.position.x, values.position.y, values.position.z, 1.f);
		}

		if (entity.HasComponent<RotationComponent>() && !(values.rotationEuler == m_TransformSnapshot.rotationEuler))
		{
			entity.GetComponent<RotationComponent>().rotation = GUI::EulerDegreesToQuaternion(values.rotationEuler);
		}

		if (entity.HasComponent<ScaleComponent>() && !(values.scale == m_TransformSnapshot.scale))
		{
			entity.GetComponent<ScaleComponent>().scale = values.scale;
		}
	}

	if (m_UiState.torus && entity.HasComponent<TorusGenerationComponent>())
	{
		const TorusValues& values = *m_UiState.torus;
		const bool changed =
			values.largeRadius != m_TorusSnapshot.largeRadius ||
			values.tubeRadius != m_TorusSnapshot.tubeRadius ||
			values.samplesU != m_TorusSnapshot.samplesU ||
			values.samplesV != m_TorusSnapshot.samplesV;

		if (changed)
		{
			auto& torus = entity.GetComponent<TorusGenerationComponent>();
			torus.radius = values.largeRadius;
			torus.tubeRadius = values.tubeRadius;
			torus.radialSegments = values.samplesU;
			torus.tubularSegments = values.samplesV;
			entity.AddTag<IsDirtyTag>();
		}
	}
}

void GUISystem::Process()
{
	SyncInspectorState();

	GUI::DrawMenuBar(m_Scene, m_UiState, m_ShowImGuiDemo, m_ShowParameterSpace, m_Callbacks);
	GUI::DrawToolbar(m_UiState);
	GUI::DrawStatusBar(m_Scene, m_Viewport.GetData(), m_UiState);

	GUI::DrawAllDialogs(m_UiState, m_Scene);
	GUI::DrawViewportContextMenu(m_Scene, m_Dockspace, m_UiState, m_ShowParameterSpace,
		m_ShowImGuiDemo, m_Callbacks);
	GUI::SyncSelectionCentreVisibility(m_Scene, m_UiState.showSelectionCentre);

	if (!m_Dockspace.Created())
	{
		GUI::Layout::Default(m_Dockspace);
	}

	m_Dockspace.ClaimSize();

	ViewportData sceneRect;
	if (m_Dockspace.TryGetCentralNodeRect(sceneRect))
	{
		m_Viewport.Change(sceneRect);
	}

	if (m_ShowImGuiDemo)
	{
		ImGui::ShowDemoWindow(&m_ShowImGuiDemo);
	}

	m_ShapeList.Display();

	GUI::DrawInspector(m_UiState, &m_InspectorCallbacks);
	WriteBackInspectorState();

	GUI::DrawCursorPanel(m_Scene, m_UiState, m_Dockspace);

	if (m_ShowParameterSpace)
	{
		GUI::DrawParameterSpace(m_UiState);
	}

	GUI::HandleViewportPicking(m_Scene, m_Dockspace, m_Picking);
	GUI::HandleCursorPlacement(m_Scene, m_Dockspace, m_UiState.cursor, m_CursorPlacement);

	GUI::HandleShortcuts(m_Scene, m_Callbacks);
	GUI::HandleFileShortcuts();
}
