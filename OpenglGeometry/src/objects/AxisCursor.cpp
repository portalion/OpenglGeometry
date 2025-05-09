#include "AxisCursor.h"

std::string AxisCursor::GetTypeName() const
{
    return std::string("AxisCursor");
}

RenderableMesh<PositionVertexData> AxisCursor::GenerateMesh()
{
    RenderableMesh<PositionVertexData> result;
    result.vertices = 
    {
        {{ 0.f, 0.f, 0.f, 1.f }},
        {{ 2.f, 0.f, 0.f, 1.f }},
        {{ 0.f, 2.f, 0.f, 1.f }},
        {{ 0.f, 0.f, 2.f, 1.f }}
    };
    result.indices = 
    {
        0, 1,
        0, 2,
        0, 3
    };
    return result;
}

IPositionComponent* AxisCursor::GetPositionComponent()
{
    return &positionComponent;
}

