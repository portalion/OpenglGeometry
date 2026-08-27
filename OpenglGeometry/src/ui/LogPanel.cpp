#include "LogPanel.h"

#include <array>
#include <vector>

#include <imgui/imgui.h>

#include "core/Log.h"

namespace
{
	ImVec4 ColorFor(Logger::Level level)
	{
		switch (level)
		{
		case Logger::Level::Trace:   return ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
		case Logger::Level::Warning: return ImVec4(0.95f, 0.77f, 0.30f, 1.0f);
		case Logger::Level::Error:   return ImVec4(0.94f, 0.40f, 0.40f, 1.0f);
		case Logger::Level::Info:
		default:                     return ImGui::GetStyleColorVec4(ImGuiCol_Text);
		}
	}

	int& MinLevelRef()
	{
		static int minLevel = static_cast<int>(Logger::Level::Trace);
		return minLevel;
	}
}

void GUI::DrawLogPanel(bool* open)
{
	if (!ImGui::Begin(LogPanelWindow, open))
	{
		ImGui::End();
		return;
	}

	static ImGuiTextFilter filter;
	static bool autoScroll = true;

	if (ImGui::Button("Clear"))
	{
		Logger::Clear();
	}

	ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &autoScroll);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	static constexpr std::array<const char*, 4> levels = { "Trace", "Info", "Warning", "Error" };
	ImGui::Combo("Min level", &MinLevelRef(), levels.data(), static_cast<int>(levels.size()));

	ImGui::SameLine();
	filter.Draw("Filter", -FLT_MIN -50);

	ImGui::Separator();

	if (ImGui::BeginChild("##LogScroll", ImVec2(0, 0), ImGuiChildFlags_None,
		ImGuiWindowFlags_HorizontalScrollbar))
	{
		const auto& entries = Logger::Entries();

		ImGuiListClipper clipper;
		std::vector<const Logger::Entry*> visible;
		visible.reserve(entries.size());

		for (const Logger::Entry& entry : entries)
		{
			if (static_cast<int>(entry.level) < MinLevelRef())
			{
				continue;
			}
			if (!filter.PassFilter(entry.message.c_str()))
			{
				continue;
			}
			visible.push_back(&entry);
		}

		clipper.Begin(static_cast<int>(visible.size()));
		while (clipper.Step())
		{
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const Logger::Entry& entry = *visible[i];

				ImGui::TextDisabled("%8.2f", entry.time);
				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Text, ColorFor(entry.level));
				if (entry.repeat > 1)
				{
					ImGui::Text("[%s] %s  (x%u)", Logger::ToString(entry.level),
						entry.message.c_str(), entry.repeat);
				}
				else
				{
					ImGui::Text("[%s] %s", Logger::ToString(entry.level), entry.message.c_str());
				}
				ImGui::PopStyleColor();
			}
		}

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}
	}
	ImGui::EndChild();

	ImGui::End();
}
