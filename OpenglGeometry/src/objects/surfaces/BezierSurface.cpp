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

	ImGui::DragInt(GenerateLabelWithId("u subdivision").c_str(), &u_subdivisions, 1, 2, 50);
	ImGui::DragInt(GenerateLabelWithId("v subdivision").c_str(), &v_subdivisions, 1, 2, 50);

    return false;
}

void BezierSurface::GeneratePlane(int xPatches, int yPatches, float sizeX, float sizeY)
{
	using pointsVector = std::vector<std::shared_ptr<Point>>;
	const unsigned int numberOfPointsX = (BezierPatchData::CONTROL_POINTS_PER_EDGE - 1) * xPatches + 1;
	const unsigned int numberOfPointsY = (BezierPatchData::CONTROL_POINTS_PER_EDGE - 1) * yPatches + 1;

	const float sizeXPerPoint = sizeX / (numberOfPointsX - 1);
	const float sizeYPerPoint = sizeY / (numberOfPointsY - 1);

	std::vector<pointsVector> points(numberOfPointsX, pointsVector(numberOfPointsY));

	for (int i = 0; i < numberOfPointsX; i++)
	{
		for (int j = 0; j < numberOfPointsY; j++)
		{
			std::shared_ptr<Point> point = std::make_shared<Point>();
			point->InitName();
			point->SetPosition({
				i * sizeXPerPoint,
				j * sizeYPerPoint,
				0.f });
			point->Update();

			shapeList->AddPoint(point);
			point->Attach(this);
			points[i][j] = point;
		}
	}

	for (int i = 0; i < xPatches; i++)
	{
		for (int j = 0; j < yPatches; j++)
		{
			BezierPatchData patch;
			for (int x = 0; x < BezierPatchData::CONTROL_POINTS_PER_EDGE; x++)
			{
				for (int y = 0; y < BezierPatchData::CONTROL_POINTS_PER_EDGE; y++)
				{
					patch.controlPoints[x][y] = points
						[i * (BezierPatchData::CONTROL_POINTS_PER_EDGE - 1) + x]
						[j * (BezierPatchData::CONTROL_POINTS_PER_EDGE - 1) + y];
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

void BezierSurface::Render() const
{
	auto shader = ShaderManager::GetInstance().GetShader(AvailableShaders::BezierSurface);
	shader->Bind();
	shader->SetUniformMat4f("u_viewMatrix", App::camera.GetViewMatrix());
	shader->SetUniformMat4f("u_projectionMatrix", App::projectionMatrix);
	shader->SetUniformVec1i("u_subdivisions", u_subdivisions);
	shader->SetUniformVec1i("v_subdivisions", v_subdivisions);
	RenderableOnScene::Render();

	auto shaderReversed = ShaderManager::GetInstance().GetShader(AvailableShaders::BezierSurfaceReversed);
	shaderReversed->Bind();
	shaderReversed->SetUniformMat4f("u_viewMatrix", App::camera.GetViewMatrix());
	shaderReversed->SetUniformMat4f("u_projectionMatrix", App::projectionMatrix);
	shaderReversed->SetUniformVec1i("u_subdivisions", u_subdivisions);
	shaderReversed->SetUniformVec1i("v_subdivisions", v_subdivisions);
	RenderableOnScene::Render();

	auto defShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);
	defShader->Bind();
}

void BezierSurface::Update(const std::string& message_from_subject)
{
	somethingChanged = true;
}

std::shared_ptr<BezierSurface> BezierSurface::Create(ShapeList* shapeList)
{
	return std::make_shared<BezierSurface>(shapeList);
}
