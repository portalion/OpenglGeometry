#include "BezierCurve.h"
#include <algorithm>
#include "App.h"

RenderableMesh<PositionVertexData> BezierCurve::GenerateMesh()
{
	controlPoints.clear();
	if (points.empty())
	{
		return {};
	}

	float minx = std::numeric_limits<float>::max();
	float miny = std::numeric_limits<float>::max(); 
	float maxx = std::numeric_limits<float>::lowest(); 
	float maxy = std::numeric_limits<float>::lowest(); 

	for (auto it = points.begin(); it != points.end();)
	{
		PositionVertexData vertex;
		if (auto ptr = it->lock())
		{
			vertex.Position = ptr->GetPosition();
			vertex.Position.w = 1.f;
			vertex.Position = App::projectionMatrix * App::camera.GetViewMatrix() * vertex.Position;
			vertex.Position = vertex.Position / vertex.Position.w;
			controlPoints.push_back(vertex.Position);

			minx = fmin(minx, vertex.Position.x);
			miny = fmin(miny, vertex.Position.y);
			maxx = fmax(maxx, vertex.Position.x);
			maxy = fmax(maxy, vertex.Position.y);
			++it;
		}
		else
		{
			it = points.erase(it);
		}
	}
	
	return RenderableMesh<PositionVertexData>
	{
		.vertices
		{
			PositionVertexData{{minx, miny, 0.f, 1.f}},
			PositionVertexData{{minx, maxy, 0.f, 1.f}},
			PositionVertexData{{maxx, miny, 0.f, 1.f}},
			PositionVertexData{{maxx, maxy, 0.f, 1.f}},
		}
	};
}

bool BezierCurve::HelperButton(ImGuiDir direction)
{
	ImGui::SameLine();
	return ImGui::ArrowButton(GenerateLabelWithId(std::to_string(static_cast<int>(direction))).c_str(), direction);
}

bool BezierCurve::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale(1.f);

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
		somethingChanged = true;
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

BezierCurve::BezierCurve(std::vector<std::shared_ptr<Point>> points, SelectedShapes* selectedShapes)
	:polyline({}), selectedShapes(selectedShapes)
{
	renderingMode = RenderingMode::PATCHES;
    for (auto& point : points)
    {
        AddPoint(point);
    }
}
