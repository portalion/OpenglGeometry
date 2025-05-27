#include "Point.h"

const RenderableMesh<PositionVertexData> Point::mesh = RenderableMesh<PositionVertexData>
{
    .vertices =
    {
        {{  0.5f,  0.5f, -0.5f, 1.f }},
        {{  0.5f, -0.5f, -0.5f, 1.f  }},
        {{ -0.5f,  0.5f, -0.5f, 1.f  }},
        {{ -0.5f, -0.5f, -0.5f, 1.f  }},
        {{  0.5f,  0.5f,  0.5f, 1.f  }},
        {{  0.5f, -0.5f,  0.5f, 1.f  }},
        {{ -0.5f,  0.5f,  0.5f, 1.f  }},
        {{ -0.5f, -0.5f,  0.5f, 1.f  }},
    },
    .indices =
    {
        0, 1, 2,  1, 3, 2,
        4, 6, 5,  5, 6, 7,
        2, 3, 6,  3, 7, 6,
        0, 4, 1,  1, 4, 5,
        0, 2, 4,  2, 6, 4,
        1, 5, 3,  3, 5, 7
    } 
};

RenderableMesh<PositionVertexData> Point::GenerateMesh()
{
    return mesh;
}

json Point::Serialize() const
{
    json result;
    result["id"] = id;
    result["name"] = name;
    auto pos = GetPosition();
    result["position"] = json::object({
        { "x", pos.x },
        { "y", pos.y },
        { "z", pos.z } });

    return result;
}

std::shared_ptr<Point> Point::Deserialize(const json& j)
{
    auto id = j["id"].get<unsigned int>();
	auto name = j["name"].get<std::string>();
    Algebra::Vector4 position;
    position.x = j["position"]["x"];
    position.y = j["position"]["y"];
    position.z = j["position"]["z"];

	auto point = std::make_shared<Point>();
	point->InitName(id, name);
	point->SetPosition(position);

    return point;
}
