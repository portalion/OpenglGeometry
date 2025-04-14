#include "BezierCurve.h"

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
			vertex.Position = ptr->GetPosition();
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

bool BezierCurve::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale(1.f);

	ImGui::Checkbox(this->GenerateLabelWithId("Draw Polyline").c_str(), &displayPolyline);

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

BezierCurve::BezierCurve(std::vector<std::shared_ptr<Point>> points)
	:polyline({})
{
    for (auto& point : points)
    {
        AddPoint(point);
    }
}
