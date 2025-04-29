#include "BezierCurveC2.h"
#include <algorithm>
#include "App.h"
#include "core/Globals.h"

RenderableMesh<PositionVertexData> BezierCurveC2::GenerateMesh()
{
	for (auto it = points.begin(); it != points.end();)
	{
		if (auto point = it->lock())
		{
			++it;
		}
		else
		{
			it = points.erase(it);
		}
	}

	bezierPoints.clear();
	bernsteinPolyline.ClearPoints();
	if (points.size() < 4) return {}; // Need at least 4 points

	for (size_t i = 0; i + 3 < points.size(); ++i) {
		const Algebra::Vector4& P0 = points[i].lock()->GetPosition();
		const Algebra::Vector4& P1 = points[i + 1].lock()->GetPosition();
		const Algebra::Vector4& P2 = points[i + 2].lock()->GetPosition();
		const Algebra::Vector4& P3 = points[i + 3].lock()->GetPosition();

		Algebra::Vector4 B0 = (P0 + 4.0f * P1 + P2) / 6.0f;
		Algebra::Vector4 B1 = (4.0f * P1 + 2.0f * P2) / 6.0f;
		Algebra::Vector4 B2 = (2.0f * P1 + 4.0f * P2) / 6.0f;
		Algebra::Vector4 B3 = (P1 + 4.0f * P2 + P3) / 6.0f;
		
		bezierPoints.push_back(std::make_shared<Point>());
		bezierPoints.back()->SetPosition(B0);
		bernsteinPolyline.AddPoint(bezierPoints.back());
		bezierPoints.push_back(std::make_shared<Point>());
		bezierPoints.back()->SetPosition(B1);
		bernsteinPolyline.AddPoint(bezierPoints.back());
		bezierPoints.push_back(std::make_shared<Point>());
		bezierPoints.back()->SetPosition(B2);
		bernsteinPolyline.AddPoint(bezierPoints.back());
		bezierPoints.push_back(std::make_shared<Point>());
		bezierPoints.back()->SetPosition(B3);
		bernsteinPolyline.AddPoint(bezierPoints.back());
	}

	return GenerateMeshFromBezier();
}

RenderableMesh<PositionVertexData> BezierCurveC2::GenerateMeshFromBezier()
{
	RenderableMesh<PositionVertexData> mesh;
	if (bezierPoints.empty())
	{
		return {};
	}

	for(const auto& ptr : bezierPoints)
	{
		PositionVertexData vertex;
		vertex.Position = ptr->GetPosition();
		vertex.Position.w = 1.f;
		mesh.vertices.push_back(vertex);
	}

	return mesh;
}

bool BezierCurveC2::HelperButton(ImGuiDir direction)
{
	ImGui::SameLine();
	return ImGui::ArrowButton(GenerateLabelWithId(std::to_string(static_cast<int>(direction))).c_str(), direction);
}

bool BezierCurveC2::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale(1.f);

	ImGui::Checkbox(this->GenerateLabelWithId("Draw Polyline").c_str(), &displayPolyline);
	ImGui::Checkbox(this->GenerateLabelWithId("Draw Bernstein Points").c_str(), &displayBezierPoints);
	ImGui::Checkbox(this->GenerateLabelWithId("Draw Bernstein Polyline").c_str(), &displayBezierPolyline);
	if (ImGui::Button(GenerateLabelWithId("Add Selected Points").c_str()))
	{
		for (const auto& selected : selectedShapes->GetSelectedWithType<Point>())
		{
			if (!selected)
				continue;

			bool alreadyExists = false;
			for (const auto& weak : points)
			{
				if (auto existing = weak.lock())
				{
					if (existing == selected)
					{
						alreadyExists = true;
						break;
					}
				}
			}

			if (!alreadyExists)
			{
				AddPoint(selected);
			}
		}
		somethingChanged = true;
	}
	ImGui::Text("Control Points (%zu)", points.size());

	int index = 0;
	for (auto it = points.begin(); it != points.end();)
	{
		if (auto point = it->lock())
		{
			ImGui::PushID(index);

			ImGui::Text(point->GetName().c_str());
			ImGui::SameLine();
			if (ImGui::Button(GenerateLabelWithId("Remove").c_str()))
			{
				polyline.RemovePoint(*it);
				it->lock()->Detach(this);
				it = points.erase(it);
				somethingChanged = true;
			}
			else
			{
				if (index > 0 && HelperButton(ImGuiDir_Up))
				{
					std::iter_swap(it, std::prev(it));
					polyline.SwapPoints(*it, *std::prev(it));
					somethingChanged = true;
				}
				if (index < points.size() - 1 && HelperButton(ImGuiDir_Down))
				{
					std::iter_swap(it, std::next(it));
					polyline.SwapPoints(*it, *std::next(it));
					somethingChanged = true;
				}
				++it;
			}

			ImGui::PopID();
			index++;
		}
		else
		{

			polyline.RemovePoint(*it);
			it = points.erase(it);
			somethingChanged = true;
		}
	}

	return somethingChanged;
}

BezierCurveC2::BezierCurveC2(std::vector<std::shared_ptr<Point>> points, SelectedShapes* selectedShapes)
	:polyline({}), selectedShapes(selectedShapes), bernsteinPolyline({})
{
	renderingMode = RenderingMode::PATCHES;
	for (auto& point : points)
	{
		AddPoint(point);
	}
}

