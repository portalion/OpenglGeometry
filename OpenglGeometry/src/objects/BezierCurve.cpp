#include "BezierCurve.h"
#include <algorithm>

RenderableMesh<PositionVertexData> BezierCurve::GenerateMesh()
{
	RenderableMesh<PositionVertexData> result;

	if (points.empty())
	{
		return result;
	}

	for (auto it = points.begin(); it != points.end();)
	{
		PositionVertexData vertex;
		if (auto ptr = it->lock())
		{
			vertex.Position = ptr->GetPositionComponent()->GetPosition();
			vertex.Position.w = 1.f;
			result.vertices.push_back(vertex);
			++it;
		}
		else
		{
			it = points.erase(it);
		}
	}

	for (int i = 0; i < result.vertices.size() - 1; i++)
	{
		result.indices.push_back(i);
		result.indices.push_back(i + 1);
	}

	return result;
}

bool BezierCurve::HelperButton(ImGuiDir direction)
{
	ImGui::SameLine();
	return ImGui::ArrowButton(GenerateLabelWithId(std::to_string(static_cast<int>(direction))).c_str(), direction);
}

bool BezierCurve::DisplayParameters()
{
	ImGui::Checkbox(this->GenerateLabelWithId("Draw Polyline").c_str(), &displayPolyline);
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
	}
    ImGui::Text("Bezier Control Points (%zu)", points.size());

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
                it = points.erase(it);
            }
            else
            {
				if (index > 0 && HelperButton(ImGuiDir_Up))
				{
					std::iter_swap(it, std::prev(it));
					polyline.SwapPoints(*it, *std::prev(it));
				}
				if (index < points.size() - 1 && HelperButton(ImGuiDir_Down))
				{
					std::iter_swap(it, std::next(it));
					polyline.SwapPoints(*it, *std::next(it));
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
        }
    }

    return true;
}

BezierCurve::BezierCurve(std::vector<std::shared_ptr<Point>> points, SelectedShapes* selectedShapes)
	:polyline({}), selectedShapes(selectedShapes)
{
    for (auto& point : points)
    {
        AddPoint(point);
    }
}
