#include "Point.h"

const RenderableMesh<PositionVertexData> Point::mesh = RenderableMesh<PositionVertexData>
{
    .vertices =
    {
        {{  1.f,  1.f, -1.f, 1.f }},
        {{  1.f, -1.f, -1.f, 1.f }},
        {{ -1.f,  1.f, -1.f, 1.f }},
        {{ -1.f, -1.f, -1.f, 1.f }},
        {{  1.f,  1.f,  1.f, 1.f }},
        {{  1.f, -1.f,  1.f, 1.f }},
        {{ -1.f,  1.f,  1.f, 1.f }},
        {{ -1.f, -1.f,  1.f, 1.f }},
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
