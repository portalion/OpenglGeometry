#include "BezierSurface.h"

RenderableMesh<PositionVertexData> BezierSurface::GenerateMesh()
{
	RenderableMesh<PositionVertexData> result;

	for (int i = 0; i < bezierPatchesData.size(); i++)
	{
		for (int j = 0; j < BezierPatchData::CONTROL_POINTS_PER_EDGE; j++)
		{
			for (int k = 0; k < BezierPatchData::CONTROL_POINTS_PER_EDGE; k++)
			{
				PositionVertexData vertex;
				vertex.Position = bezierPatchesData[i].controlPoints[j][k]->GetPosition();
				vertex.Position.w = 1.f;
				result.vertices.push_back(vertex);
			}
		}
	}

    return result;
}

bool BezierSurface::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale(1.f);

    return false;
}

void BezierSurface::GeneratePlane(int xPatches, int yPatches, float sizeX, float sizeY)
{
	const float sizeXPerPatch = sizeX / xPatches;
	const float sizeYPerPatch = sizeY / yPatches;
	const float sizeXPerPoint = sizeXPerPatch / (BezierPatchData::CONTROL_POINTS_PER_EDGE - 1);
	const float sizeYPerPoint = sizeYPerPatch / (BezierPatchData::CONTROL_POINTS_PER_EDGE - 1);
	for (int i = 0; i < xPatches; i++)
	{
		for (int j = 0; j < yPatches; j++)
		{
			BezierPatchData patch;
			for (int x = 0; x < BezierPatchData::CONTROL_POINTS_PER_EDGE; x++)
			{
				for (int y = 0; y < BezierPatchData::CONTROL_POINTS_PER_EDGE; y++)
				{
					std::shared_ptr<Point> point = std::make_shared<Point>();
					point->InitName();
					point->SetPosition({
						i * sizeXPerPatch + x * sizeXPerPoint,
						j * sizeYPerPatch + y * sizeXPerPoint,
						10.f * (float)x + y });
					point->Update();

					point->Attach(this);
					shapeList->AddPoint(point);

					patch.controlPoints[x][y] = point;
				}
			}
			bezierPatchesData.push_back(patch);
		}
	}
}

BezierSurface::BezierSurface(ShapeList* shapeList)
	:shapeList{shapeList}
{
	renderingMode = RenderingMode::PATCHES;
	GeneratePlane(1, 1, 30.f, 30.f);
}

BezierSurface::~BezierSurface()
{
	for (auto patch : bezierPatchesData)
	{
		for (int i = 0; i < BezierPatchData::CONTROL_POINTS_PER_EDGE; i++)
		{
			for (int j = 0; j < BezierPatchData::CONTROL_POINTS_PER_EDGE; j++)
			{
				patch.controlPoints[i][j]->Detach(this);
				shapeList->RemovePoint(patch.controlPoints[i][j]);
			}
		}
	}
}

void BezierSurface::Update(const std::string& message_from_subject)
{
	somethingChanged = true;
}
