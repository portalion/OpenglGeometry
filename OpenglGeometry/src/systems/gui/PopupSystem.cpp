#include "PopupSystem.h"
#include <imgui/imgui.h>
#include <UI/popups/ShapeCreation.h>

PopupSystem::PopupSystem(Ref<Scene> scene)
{
	m_RegisteredPopups.push_back(CreateRef<ShapeCreation>(scene));
}

void PopupSystem::Process()
{
	for (auto& popup : m_RegisteredPopups)
	{
		if (popup->ShouldOpen())
		{
			ImGui::OpenPopup(popup->Name());
			m_OpenedPopup = popup;
			break;
		}
	}

	if (m_OpenedPopup != nullptr &&
		ImGui::BeginPopup(m_OpenedPopup->Name()))
	{
		m_OpenedPopup->Display();
		ImGui::EndPopup();
	}
	else
	{
		m_OpenedPopup = nullptr;
	}
}
