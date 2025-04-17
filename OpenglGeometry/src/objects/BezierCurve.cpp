#include "BezierCurve.h"
#include <algorithm>
#include "App.h"
#include "core/Globals.h"

RenderableMesh<PositionVertexData> BezierCurve::GenerateMesh()
{
	RenderableMesh<PositionVertexData> mesh;
	if (points.empty())
	{
		return {};
	}

	float minx = std::numeric_limits<float>::max();
	float miny = std::numeric_limits<float>::max(); 
	float maxx = std::numeric_limits<float>::lowest(); 
	float maxy = std::numeric_limits<float>::lowest(); 

	int i = 0;
	for (auto it = points.begin(); it != points.end();)
	{
		PositionVertexData vertex;
		if (auto ptr = it->lock())
		{
			vertex.Position = ptr->GetPosition();
			vertex.Position.w = 1.f;
			mesh.vertices.push_back(vertex);
			if (++i % 4 == 0)
			{
				mesh.vertices.push_back(vertex);
				i++;
			}

			vertex.Position = App::projectionMatrix * App::camera.GetViewMatrix() * vertex.Position;
			vertex.Position = vertex.Position / vertex.Position.w;

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
	int fix = mesh.vertices.size() % 4;
	if (fix == 1) {
		// Duplicate the single point to create a degenerate cubic
		auto p = mesh.vertices[mesh.vertices.size() - 1];
		mesh.vertices.pop_back();
		mesh.vertices.push_back(p);
		mesh.vertices.push_back(p);
		mesh.vertices.push_back(p);
		mesh.vertices.push_back(p);
	}
	else if (fix == 2) {
		// Create a straight line cubic: start and end duplicated
		auto p0 = mesh.vertices[mesh.vertices.size() - 2];
		auto p1 = mesh.vertices[mesh.vertices.size() - 1];
		mesh.vertices.pop_back();
		mesh.vertices.pop_back();
		mesh.vertices.push_back(p0);
		mesh.vertices.push_back(p0);
		mesh.vertices.push_back(p1);
		mesh.vertices.push_back(p1);
	}
	else if (fix == 3) {
		// Upgrade quadratic to cubic
		auto p0 = mesh.vertices[mesh.vertices.size() - 3];
		auto p1 = mesh.vertices[mesh.vertices.size() - 2];
		auto p2 = mesh.vertices[mesh.vertices.size() - 1];
		mesh.vertices.pop_back();
		mesh.vertices.pop_back();
		mesh.vertices.pop_back();
		mesh.vertices.push_back(p0);
		mesh.vertices.push_back(PositionVertexData{ .Position {(1.0f / 3.0f) * p0.Position + (2.0f / 3.0f) * p1.Position} });
		mesh.vertices.push_back(PositionVertexData{ .Position {(2.0f / 3.0f) * p1.Position + (1.0f / 3.0f) * p2.Position} });
		mesh.vertices.push_back(p2);
	}
	sizeInPixels = static_cast<int>(static_cast<float>(Globals::startingSceneWidth) * (maxx - minx) + static_cast<float>(Globals::startingSceneHeight) * (maxy - miny)) * points.size();
	
	return mesh;
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
