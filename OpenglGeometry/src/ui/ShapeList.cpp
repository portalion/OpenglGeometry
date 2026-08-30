#include "ShapeList.h"

#include <algorithm>
#include <cctype>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include "SceneActions.h"
#include "Utils.h"

namespace
{
	std::string ToLower(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		return text;
	}

	void SetInvisibleTag(Entity entity, bool hidden)
	{
		if (!entity.IsValid() || hidden == entity.HasComponent<IsInvisibleTag>())
		{
			return;
		}

		if (hidden)
		{
			entity.AddTag<IsInvisibleTag>();
		}
		else
		{
			entity.RemoveTag<IsInvisibleTag>();
		}
	}

	void SetHidden(Entity entity, bool hidden)
	{
		SetInvisibleTag(entity, hidden);

		if (entity.HasComponent<IsParentOfVirtualEntitiesComponent>())
		{
			for (Entity virtualEntity : entity.GetComponent<IsParentOfVirtualEntitiesComponent>().virtualEntities)
			{
				SetInvisibleTag(virtualEntity, hidden);
			}
		}
	}
}

GUI::ShapeList::ShapeList(Ref<Scene> scene)
	: m_Scene{ scene }
{
	m_VisibleTypes.fill(true);
}

void GUI::ShapeList::RequestRename()
{
	m_RenameRequested = true;
}

void GUI::ShapeList::Display()
{
	ImGui::Begin(ShapeListWindow);

	if (m_RenameRequested)
	{
		m_RenameRequested = false;
		BeginRename(SingleSelected(m_Scene));
	}

	if (m_RenameTarget.IsValid() && !m_RenameTarget.HasComponent<NameComponent>())
	{
		CancelRename();
	}

	DrawActionRow();
	DrawFilterRow();
	ImGui::Separator();

	std::array<std::vector<Entity>, ObjectTypeCount> grouped;
	std::size_t total = 0;

	for (Entity entity : GetSceneObjects(m_Scene))
	{
		const auto type = entity.GetComponent<ObjectTypeComponent>().type;
		total++;

		const bool isBeingRenamed = m_RenameTarget.IsValid() && m_RenameTarget == entity;

		if (!isBeingRenamed && (!IsVisible(type) || !PassesFilter(entity)))
		{
			continue;
		}

		grouped[static_cast<std::size_t>(type)].push_back(entity);
	}

	if (total == 0)
	{
		ImGui::TextDisabled("No objects in the scene.");
		ImGui::End();
		return;
	}

	if (ImGui::BeginChild("##ShapeListItems", ImVec2(0.f, 0.f), ImGuiChildFlags_None))
	{
		bool anythingShown = false;

		for (ObjectType type : AllObjectTypes)
		{
			const auto& entities = grouped[static_cast<std::size_t>(type)];
			if (entities.empty())
			{
				continue;
			}

			anythingShown = true;
			DrawGroup(type, entities);
		}

		if (!anythingShown)
		{
			ImGui::TextDisabled("No objects match the filter.");
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void GUI::ShapeList::DrawActionRow()
{
	const bool anythingSelected = AnythingSelected(m_Scene);

	if (ImGui::Button("Select All##Shape List"))
	{
		SelectAll(m_Scene);
	}

	ImGui::SameLine();

	ImGui::BeginDisabled(!anythingSelected);
	if (ImGui::Button("Deselect All##Shape List"))
	{
		DeselectAll(m_Scene);
	}
	ImGui::EndDisabled();

	ImGui::BeginDisabled(!anythingSelected);
	if (ImGui::Button("Remove Selected##Shape List"))
	{
		DeleteSelected(m_Scene);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Focus##Shape List"))
	{
		FocusSelected(m_Scene);
	}

	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Orbit around the selection, or the cursor when nothing is selected (F)");
	}

	ImGui::SameLine();

	ImGui::BeginDisabled(!CanRename(m_Scene));
	if (ImGui::Button("Rename##Shape List"))
	{
		RequestRename();
	}
	ImGui::EndDisabled();

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("Rename the selected object (F2)");
	}
}

void GUI::ShapeList::DrawFilterRow()
{
	const float typeButtonWidth = ImGui::CalcTextSize("Types").x + ImGui::GetStyle().FramePadding.x * 4.f;

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - typeButtonWidth - ImGui::GetStyle().ItemSpacing.x);
	ImGui::InputTextWithHint("##Shape List Filter", "Filter by name or id", &m_Filter);

	ImGui::SameLine();

	const bool allTypesVisible = std::ranges::all_of(m_VisibleTypes, [](bool visible) { return visible; });

	if (!allTypesVisible)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
	}

	if (ImGui::Button("Types##Shape List"))
	{
		ImGui::OpenPopup("##Shape List Types");
	}

	if (!allTypesVisible)
	{
		ImGui::PopStyleColor();
	}

	if (ImGui::BeginPopup("##Shape List Types"))
	{
		if (ImGui::SmallButton("All##Shape List Types"))
		{
			m_VisibleTypes.fill(true);
		}

		ImGui::SameLine();

		if (ImGui::SmallButton("None##Shape List Types"))
		{
			m_VisibleTypes.fill(false);
		}

		ImGui::Separator();

		for (ObjectType type : AllObjectTypes)
		{
			bool& visible = m_VisibleTypes[static_cast<std::size_t>(type)];
			ImGui::Checkbox(ToDisplayPluralString(type), &visible);
		}

		ImGui::EndPopup();
	}
}

void GUI::ShapeList::DrawGroup(ObjectType type, const std::vector<Entity>& entities)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

	if (type == ObjectType::Point && m_Filter.empty())
	{
		flags &= ~ImGuiTreeNodeFlags_DefaultOpen;
	}

	const bool containsRenameTarget = m_RenameTarget.IsValid()
		&& std::ranges::any_of(entities, [this](Entity entity) { return entity == m_RenameTarget; });

	if (!m_Filter.empty() || containsRenameTarget)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	}

	if (!ImGui::TreeNodeEx(ToDisplayPluralString(type), flags, "%s (%zu)",
		ToDisplayPluralString(type), entities.size()))
	{
		return;
	}

	for (Entity entity : entities)
	{
		DrawRow(entity);
	}

	ImGui::TreePop();
}

void GUI::ShapeList::DrawRow(Entity entity)
{
	if (m_RenameTarget.IsValid() && m_RenameTarget == entity)
	{
		DrawRenameField(entity);
		return;
	}

	const bool isSelected = entity.HasComponent<IsSelectedTag>();
	const bool visible = !entity.HasComponent<IsInvisibleTag>();
	const auto& name = entity.GetComponent<NameComponent>().name;

	bool toggled = visible;
	if (ImGui::Checkbox(GenerateLabel(entity, "##visible").c_str(), &toggled))
	{
		SetHidden(entity, !toggled);
	}

	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(visible ? "Hide" : "Show");
	}

	ImGui::SameLine();

	if (!visible)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
	}

	if (ImGui::Selectable(GenerateLabel(entity, name).c_str(), isSelected,
		ImGuiSelectableFlags_AllowDoubleClick))
	{
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			SelectOnly(m_Scene, entity);
			FocusSelected(m_Scene);
		}
		else if (isSelected)
		{
			entity.RemoveTag<IsSelectedTag>();
		}
		else
		{
			entity.AddTag<IsSelectedTag>();
		}
	}

	if (!visible)
	{
		ImGui::PopStyleColor();
	}

	DrawRowContextMenu(entity);

	ImGui::SameLine();
	TextDisabledRightAligned(("#" + std::to_string(entity.GetComponent<IdComponent>().id)).c_str());
}

void GUI::ShapeList::DrawRowContextMenu(Entity entity)
{
	if (!ImGui::BeginPopupContextItem(GenerateLabel(entity, "##context").c_str()))
	{
		return;
	}

	ImGui::TextDisabled("%s  #%u",
		ToDisplayString(entity.GetComponent<ObjectTypeComponent>().type),
		entity.GetComponent<IdComponent>().id);
	ImGui::Separator();

	if (ImGui::MenuItem("Rename", "F2"))
	{
		SelectOnly(m_Scene, entity);
		BeginRename(entity);
	}
	if (ImGui::MenuItem("Focus", "F"))
	{
		SelectOnly(m_Scene, entity);
		FocusSelected(m_Scene);
	}

	const bool hidden = entity.HasComponent<IsInvisibleTag>();
	if (ImGui::MenuItem(hidden ? "Show" : "Hide"))
	{
		SetHidden(entity, !hidden);
	}

	ImGui::Separator();
	const bool lockedPoint = IsSurfaceControlPoint(entity);
	if (ImGui::MenuItem("Delete", "Del", false, !lockedPoint))
	{
		entity.AddTag<ToBeDestroyedTag>();
	}
	if (lockedPoint && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("a surface's control points cannot be deleted");
	}

	ImGui::EndPopup();
}

void GUI::ShapeList::DrawRenameField(Entity entity)
{
	if (m_RenameNeedsFocus)
	{
		ImGui::SetKeyboardFocusHere();
		m_RenameNeedsFocus = false;
	}

	ImGui::SetNextItemWidth(-FLT_MIN);

	const bool committed = ImGui::InputText(GenerateLabel(entity, "##rename").c_str(), &m_RenameBuffer,
		ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

	if (committed || ImGui::IsItemDeactivatedAfterEdit())
	{
		CommitRename();
	}
	else if (ImGui::IsItemDeactivated())
	{
		CancelRename();
	}
}

bool GUI::ShapeList::PassesFilter(Entity entity) const
{
	if (m_Filter.empty())
	{
		return true;
	}

	const std::string needle = ToLower(m_Filter);

	if (ToLower(entity.GetComponent<NameComponent>().name).find(needle) != std::string::npos)
	{
		return true;
	}

	return std::to_string(entity.GetComponent<IdComponent>().id).find(needle) != std::string::npos;
}

bool GUI::ShapeList::IsVisible(ObjectType type) const
{
	return m_VisibleTypes[static_cast<std::size_t>(type)];
}

void GUI::ShapeList::BeginRename(Entity entity)
{
	if (!entity.IsValid() || !entity.HasComponent<NameComponent>())
	{
		return;
	}

	m_RenameTarget = entity;
	m_RenameBuffer = entity.GetComponent<NameComponent>().name;
	m_RenameNeedsFocus = true;
}

void GUI::ShapeList::CommitRename()
{
	if (m_RenameTarget.IsValid() && m_RenameTarget.HasComponent<NameComponent>() && !m_RenameBuffer.empty())
	{
		m_RenameTarget.GetComponent<NameComponent>().name = m_RenameBuffer;
	}

	CancelRename();
}

void GUI::ShapeList::CancelRename()
{
	m_RenameTarget = Entity{};
	m_RenameBuffer.clear();
	m_RenameNeedsFocus = false;
}
