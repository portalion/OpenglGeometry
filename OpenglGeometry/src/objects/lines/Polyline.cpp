#include "Polyline.h"
#include <UI/ShapeList.h>

RenderableMesh<PositionVertexData> Polyline::GenerateMesh()
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
	
	for (int i = 0; i < static_cast<int>(result.vertices.size()) - 1; i++)
	{
		result.indices.push_back(i);
		result.indices.push_back(i + 1);
	}	

    return result;
}

bool Polyline::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale(1.f);

    ImGui::Text("Polyline Points (%zu)", points.size());

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
                somethingChanged = true;
                point->Detach(this);
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
            somethingChanged = true;
            it = points.erase(it);
        }
    }
   
    return somethingChanged;
}

Polyline::Polyline(std::vector<std::shared_ptr<Point>> points)
{
    for (auto& point : points)
    {
        AddPoint(point);
    }
}

std::shared_ptr<Polyline> Polyline::Deserialize(const json& j, ShapeList* list)
{
	auto id = j["id"].get<unsigned int>();
	auto name = j["name"].get<std::string>();
	std::vector<std::shared_ptr<Point>> points;
	for (const auto& pointJson : j["controlPoints"])
	{
		auto pointId = pointJson["id"].get<unsigned int>();
		auto point = list->GetPointWithId(pointId);
		if (point)
		{
			points.push_back(point);
		}
	}
	std::reverse(points.begin(), points.end()); 
	auto polyline = std::make_shared<Polyline>(points);
	polyline->InitName(id, name);
    return polyline;
}

json Polyline::Serialize() const
{
    json result;
	result["objectType"] = "chain";
	result["id"] = id;
	result["name"] = name;
	result["controlPoints"] = json::array();
	for (const auto& point : points)
	{
		if (auto ptr = point.lock())
		{
            result["controlPoints"].push_back(json::object({ { "id", ptr->GetShapeId() } }));
		}
	}

    return result;
}
