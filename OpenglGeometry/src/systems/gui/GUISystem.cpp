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

	Entity FindEntityById(Ref<Scene> scene, uint32_t id)
	{
		for (Entity entity : scene->GetAllEntitiesWith<IdComponent>())
		{
			if (entity.GetComponent<IdComponent>().id == id)
			{
				return entity;
			}
		}
		return Entity{};
	}

	TransformValues ReadTransform(Entity entity)
	{
		TransformValues values;
		values.position = entity.GetComponent<PositionComponent>().position;

		if (entity.HasComponent<RotationComponent>())
		{
			values.rotationEuler = GUI::QuaternionToEulerDegrees(entity.GetComponent<RotationComponent>().rotation);
		}

		values.scale = entity.HasComponent<ScaleComponent>()
			? entity.GetComponent<ScaleComponent>().scale
			: Algebra::Vector4(1.f, 1.f, 1.f, 0.f);

		return values;
	}

	void WriteTransform(Entity entity, const TransformValues& values)
	{
		if (!entity.IsValid() || !entity.HasComponent<PositionComponent>())
		{
			return;
		}

		const Algebra::Vector4 wantPosition(values.position.x, values.position.y, values.position.z, 1.f);
		const Algebra::Vector4 currentPosition = entity.GetComponent<PositionComponent>().position;
		if (!(currentPosition == wantPosition))
		{
			entity.GetComponent<PositionComponent>().position = wantPosition;
		}

		if (entity.HasComponent<RotationComponent>())
		{
			const Algebra::Vector4 currentEuler =
				GUI::QuaternionToEulerDegrees(entity.GetComponent<RotationComponent>().rotation);
			if (!(currentEuler == values.rotationEuler))
			{
				entity.GetComponent<RotationComponent>().rotation = GUI::EulerDegreesToQuaternion(values.rotationEuler);
			}
		}

		if (entity.HasComponent<ScaleComponent>() && !(entity.GetComponent<ScaleComponent>().scale == values.scale))
		{
			entity.GetComponent<ScaleComponent>().scale = values.scale;
		}
	}

	TorusValues ReadTorus(Entity entity)
	{
		const auto& torus = entity.GetComponent<TorusGenerationComponent>();
		TorusValues values;
		values.largeRadius = torus.radius;
		values.tubeRadius = torus.tubeRadius;
		values.samplesU = torus.radialSegments;
		values.samplesV = torus.tubularSegments;
		return values;
	}

	void WriteTorus(Entity entity, const TorusValues& values)
	{
		if (!entity.IsValid() || !entity.HasComponent<TorusGenerationComponent>())
		{
			return;
		}

		auto& torus = entity.GetComponent<TorusGenerationComponent>();
		if (torus.radius == values.largeRadius && torus.tubeRadius == values.tubeRadius
			&& torus.radialSegments == values.samplesU && torus.tubularSegments == values.samplesV)
		{
			return;
		}

		torus.radius = values.largeRadius;
		torus.tubeRadius = values.tubeRadius;
		torus.radialSegments = values.samplesU;
		torus.tubularSegments = values.samplesV;
		entity.AddTag<IsDirtyTag>();
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

		if (row.selected)
		{
			if (entity.HasComponent<PositionComponent>())
			{
				row.transform = ReadTransform(entity);
			}
			if (entity.HasComponent<TorusGenerationComponent>())
			{
				row.torus = ReadTorus(entity);
			}
		}

		m_UiState.objects.push_back(row);
	}

	m_UiState.cursor.selectionCentre = GUI::SelectionCentre(m_Scene);

	m_UiState.transform.reset();
	m_UiState.torus.reset();
	m_UiState.curve.reset();
	m_UiState.surface.reset();

	const ObjectRow* onlySelected = nullptr;
	int selectedCount = 0;
	for (const ObjectRow& row : m_UiState.objects)
	{
		if (row.selected)
		{
			selectedCount++;
			onlySelected = &row;
		}
	}

	if (selectedCount == 1 && onlySelected)
	{
		m_UiState.transform = onlySelected->transform;
		m_UiState.torus = onlySelected->torus;
	}
}

void GUISystem::WriteBackInspectorState()
{
	const ObjectRow* onlySelected = nullptr;
	int selectedCount = 0;

	for (const ObjectRow& row : m_UiState.objects)
	{
		if (!row.selected)
		{
			continue;
		}

		selectedCount++;
		onlySelected = &row;

		Entity entity = FindEntityById(m_Scene, row.id);
		if (!entity.IsValid())
		{
			continue;
		}

		if (entity.HasComponent<NameComponent>() && entity.GetComponent<NameComponent>().name != row.name)
		{
			entity.GetComponent<NameComponent>().name = row.name;
		}

		if (row.transform)
		{
			WriteTransform(entity, *row.transform);
		}
		if (row.torus)
		{
			WriteTorus(entity, *row.torus);
		}
	}

	if (selectedCount == 1 && onlySelected)
	{
		Entity entity = FindEntityById(m_Scene, onlySelected->id);
		if (m_UiState.transform)
		{
			WriteTransform(entity, *m_UiState.transform);
		}
		if (m_UiState.torus)
		{
			WriteTorus(entity, *m_UiState.torus);
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
