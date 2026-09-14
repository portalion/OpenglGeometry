#include "ParameterSpace.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
#include <imgui/imgui.h>

#include "Widgets.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/Tags.h"
#include "scene/ObjectType.h"
#include "archetypes/Archetypes.h"
#include "core/Log.h"

namespace
{
	Entity SelectedIntersectionCurve(Ref<Scene> scene)
	{
		if (!scene)
		{
			return Entity{};
		}
		for (Entity entity : scene->GetAllEntitiesWith<IsSelectedTag, IntersectionCurveComponent>())
		{
			return entity;
		}
		for (Entity entity : scene->GetAllEntitiesWith<IntersectionCurveComponent>())
		{
			return entity;
		}
		return Entity{};
	}

	void DrawParamCurve(const char* id, const char* label,
		const std::vector<Algebra::Vector4>& params, bool wrappedU, bool wrappedV)
	{
		ImGui::BeginGroup();
		ImGui::TextUnformatted(label);

		ImGui::PushID(id);
		ImGui::BeginChild("##Square", ImVec2(260.f, 260.f), ImGuiChildFlags_Border);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 origin = ImGui::GetWindowPos();
		const ImVec2 size = ImGui::GetWindowSize();

		const auto toPixel = [&](float u, float v)
		{
			return ImVec2(origin.x + u * size.x, origin.y + (1.f - v) * size.y);
		};

		drawList->AddRectFilled(origin, ImVec2(origin.x + size.x, origin.y + size.y),
			IM_COL32(11, 12, 15, 255));

		for (int i = 1; i < 4; i++)
		{
			const float t = static_cast<float>(i) / 4.f;
			drawList->AddLine(toPixel(t, 0.f), toPixel(t, 1.f), IM_COL32(30, 34, 40, 255));
			drawList->AddLine(toPixel(0.f, t), toPixel(1.f, t), IM_COL32(30, 34, 40, 255));
		}

		const ImU32 lineColor = IM_COL32(38, 240, 140, 235);
		for (std::size_t i = 1; i < params.size(); i++)
		{
			const Algebra::Vector4& a = params[i - 1];
			const Algebra::Vector4& b = params[i];

			const bool jumpU = wrappedU && std::fabs(a.x - b.x) > 0.5f;
			const bool jumpV = wrappedV && std::fabs(a.y - b.y) > 0.5f;
			if (jumpU || jumpV)
			{
				continue;
			}

			drawList->AddLine(toPixel(a.x, a.y), toPixel(b.x, b.y), lineColor, 1.6f);
		}

		for (const Algebra::Vector4& p : params)
		{
			drawList->AddCircleFilled(toPixel(p.x, p.y), 1.6f, lineColor);
		}

		ImGui::EndChild();
		ImGui::PopID();
		ImGui::EndGroup();
	}

	void ConvertToSpline(Ref<Scene> scene, Entity curve)
	{
		const auto& data = curve.GetComponent<IntersectionCurveComponent>();
		if (data.points.size() < 3)
		{
			Logger::Warning("Convert: intersection curve has too few points");
			return;
		}

		constexpr std::size_t target = 24;
		const std::size_t stride = std::max<std::size_t>(1, data.points.size() / target);

		std::vector<Entity> points;
		for (std::size_t i = 0; i < data.points.size(); i += stride)
		{
			points.push_back(Archetypes::CreatePoint(scene.get(), data.points[i]));
		}
		if (!data.closed && (data.points.size() - 1) % stride != 0)
		{
			points.push_back(Archetypes::CreatePoint(scene.get(), data.points.back()));
		}

		if (points.size() < 2)
		{
			Logger::Warning("Convert: not enough sample points");
			return;
		}

		Archetypes::CreateInterpolatedBezier(scene.get(), points.begin(), points.end());
		Logger::Info("Converted intersection curve to an interpolating C2 curve ({} points)", points.size());
	}
}

void GUI::DrawParameterSpace(Ref<Scene> scene, UiState&)
{
	ImGui::Begin(ParameterSpaceWindow);

	Entity curve = SelectedIntersectionCurve(scene);
	if (!curve.IsValid())
	{
		ImGui::TextWrapped("Select two surfaces (or one, for a self-intersection) and use "
			"Create > Intersection curve. The traced curve shows here in the parameter "
			"space of each surface.");
		ImGui::End();
		return;
	}

	auto& data = curve.GetComponent<IntersectionCurveComponent>();
	
	DrawParamCurve("##SquareP", data.surfaceP.GetComponent<NameComponent>().name.c_str(), data.paramsP, data.wrappedPU, data.wrappedPV);
	if (data.surfaceQ.IsValid())
	{
		ImGui::SameLine();
		DrawParamCurve("##SquareQ", data.surfaceQ.GetComponent<NameComponent>().name.c_str(), data.paramsQ, data.wrappedQU, data.wrappedQV);
	}

	ImGui::SameLine();
	ImGui::BeginGroup();

	ImGui::Text("%s, %d points", data.closed ? "closed" : "open",
		static_cast<int>(data.points.size()));

	float stepLength = data.stepLength;
	float precision = data.precision;

	bool changed = false;
	if (BeginPropertyTable("##ParamSpace"))
	{
		changed |= PropertyRow("Step length", stepLength, 0.002f, 0.005f, 0.25f, "%.3f");
		changed |= PropertyRow("Precision", precision, 0.00005f, 1e-5f, 1e-2f, "%.5f");
		EndPropertyTable();
	}

	if (changed)
	{
		data.stepLength = std::clamp(stepLength, 0.005f, 0.25f);
		data.precision = std::clamp(precision, 1e-5f, 1e-2f);
		data.retraceRequested = true;
		curve.AddTag<IsDirtyTag>();
	}

	if (ImGui::Button("Re-trace"))
	{
		data.retraceRequested = true;
		curve.AddTag<IsDirtyTag>();
	}

	ImGui::SameLine();
	if (ImGui::Button("Convert to interpolating C2"))
	{
		ConvertToSpline(scene, curve);
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Trimming");

	const auto trimControls = [&](Entity surface, const char* fallback)
	{
		if (!surface.IsValid() || !surface.HasComponent<TrimmingComponent>())
		{
			return;
		}
		auto& trimming = surface.GetComponent<TrimmingComponent>();
		const std::string name = surface.HasComponent<NameComponent>()
			? surface.GetComponent<NameComponent>().name : std::string(fallback);

		ImGui::PushID(name.c_str());
		bool enabled = trimming.enabled && trimming.activeCurve == curve;
		if (ImGui::Checkbox(("Trim " + name).c_str(), &enabled))
		{
			trimming.enabled = enabled;
			if (enabled)
			{
				trimming.activeCurve = curve;
			}
			trimming.maskDirty = true;
		}
		if (trimming.enabled && trimming.activeCurve == curve)
		{
			int side = trimming.side;
			ImGui::SameLine();
			ImGui::RadioButton("keep filled", &side, 0);
			ImGui::SameLine();
			ImGui::RadioButton("keep other", &side, 1);
			if (side != trimming.side)
			{
				trimming.side = side;
			}
		}
		ImGui::PopID();
	};

	trimControls(data.surfaceP, data.surfaceP.GetComponent<NameComponent>().name.c_str());
	if (!data.selfIntersection)
	{
		trimControls(data.surfaceQ, data.surfaceQ.GetComponent<NameComponent>().name.c_str());
	}
	if ((!data.surfaceP.IsValid() || !data.surfaceP.HasComponent<TrimmingComponent>())
		&& (!data.surfaceQ.IsValid() || !data.surfaceQ.HasComponent<TrimmingComponent>()))
	{
		ImGui::TextDisabled("trimming is available for Bezier surfaces only");
	}

	ImGui::EndGroup();

	ImGui::End();
}
