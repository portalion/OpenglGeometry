#include "BezierSurface.h"
#include "core/Globals.h"

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

	ImGui::Checkbox(GenerateLabelWithId("Draw Bezier Polygon").c_str(), &drawBezierPolygon);

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

void BezierSurface::GenerateCylinder(int radiusPatches, int heightPatches, float radius, float height)
{
	const int columns = radiusPatches * 3;
	const int rows = heightPatches * 3 + 1;

	float dHeight = height / static_cast<float>(heightPatches * 3);
	float dAngle = 2.f * std::numbers::pi_v<float> / static_cast<float>(columns);

	Algebra::Vector4 startingPosition = Algebra::Vector4();

	std::vector<std::shared_ptr<Point>> controlPoints(rows * columns);
	int k = 0;
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < columns; ++j)
		{
			Algebra::Vector4 heightOffset = Algebra::Vector4(0.f, 0.f, i * dHeight);
			Algebra::Vector4 radiusOffset = Algebra::Matrix4::RotationZ(dAngle * j) * Algebra::Vector4(radius, 0.f, 0.f);

			auto point = std::make_shared<Point>();
			point->InitName();
			point->SetPosition(startingPosition + heightOffset + radiusOffset);
			point->Update();

			shapeList->AddPoint(point);
			point->Attach(this);
			controlPoints[k++] = point;
		}
	}

	for (int patchIndex = 0; patchIndex < radiusPatches * heightPatches; ++patchIndex)
	{
		BezierPatchData patch;

		int startingI = patchIndex / radiusPatches;
		int startingJ = patchIndex % radiusPatches;

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				int index = (startingI * 3 + i) * columns + (startingJ * 3 + j) % columns;
				patch.controlPoints[i][j] = controlPoints[index];
			}
		}

		bezierPatchesData.push_back(patch);
	}
}

void AddPolygons(std::vector<std::shared_ptr<Polyline>>& polygons, const BezierPatchData& patch)
{
	std::vector<std::shared_ptr<Point>> controlPoints;
	for (int i = 0; i < BezierPatchData::CONTROL_POINTS_PER_EDGE; i++)
	{
		controlPoints.clear();
		for (int j = 0; j < BezierPatchData::CONTROL_POINTS_PER_EDGE; j++)
		{
			controlPoints.push_back(patch.controlPoints[i][j]);
		}
		polygons.push_back(std::make_shared<Polyline>(controlPoints));
	}
	for (int i = 0; i < BezierPatchData::CONTROL_POINTS_PER_EDGE; i++)
	{
		controlPoints.clear();
		for (int j = 0; j < BezierPatchData::CONTROL_POINTS_PER_EDGE; j++)
		{
			controlPoints.push_back(patch.controlPoints[j][i]);
		}
		polygons.push_back(std::make_shared<Polyline>(controlPoints));
	}
}

BezierSurface::BezierSurface(ShapeList* shapeList, bool isCylinder, float sizex, float sizey, int xpatch, int ypatch)
	:shapeList{shapeList}
{
	u_patches = xpatch;
	v_patches = ypatch;
	renderingMode = RenderingMode::PATCHES;
	if (isCylinder)
	{
		GenerateCylinder(xpatch, ypatch, sizex, sizey);
	}
	else
	{
		GeneratePlane(xpatch, ypatch, sizex, sizey);
	}

	for (int i = 0; i < bezierPatchesData.size(); i++)
	{
		AddPolygons(bezierPolygon, bezierPatchesData[i]);
	}
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
	for (auto polygon : bezierPolygon)
		polygon->Update();
	auto shader = ShaderManager::GetInstance().GetShader(AvailableShaders::BezierSurface);
	shader->Bind();
	shader->SetUniformMat4f("u_viewMatrix", App::camera.GetViewMatrix());
	shader->SetUniformMat4f("u_projectionMatrix", App::projectionMatrix);
	shader->SetUniformVec1i("u_subdivisions", u_subdivisions);
	shader->SetUniformVec1i("v_subdivisions", v_subdivisions);
	shader->SetUniformVec4f("u_color", Globals::defaultLineColor);
	RenderableOnScene::Render();

	auto shaderReversed = ShaderManager::GetInstance().GetShader(AvailableShaders::BezierSurfaceReversed);
	shaderReversed->Bind();
	shaderReversed->SetUniformMat4f("u_viewMatrix", App::camera.GetViewMatrix());
	shaderReversed->SetUniformMat4f("u_projectionMatrix", App::projectionMatrix);
	shaderReversed->SetUniformVec1i("u_subdivisions", u_subdivisions);
	shaderReversed->SetUniformVec1i("v_subdivisions", v_subdivisions);
	shaderReversed->SetUniformVec4f("u_color", Globals::defaultLineColor);

	RenderableOnScene::Render();

	auto defShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);
	defShader->Bind();

	if(drawBezierPolygon)
	{
		for (auto polygon : bezierPolygon)
			polygon->Render();
	}
}

void BezierSurface::Update(const std::string& message_from_subject)
{
	somethingChanged = true;
}

std::shared_ptr<BezierSurface> BezierSurface::Create(ShapeList* shapeList, bool isCylinder, float sizex, float sizey, int xpatch, int ypatch)
{
	return std::make_shared<BezierSurface>(shapeList, isCylinder, sizex, sizey, xpatch, ypatch);
}

json BezierSurface::Serialize() const
{
	json result;
	result["objectType"] = "bezierSurfaceC0";
	result["id"] = id;
	result["name"] = name;
	result["controlPoints"] = json::array();

	for (int j = 0; j < BezierPatchData::CONTROL_POINTS_PER_EDGE; j++)
	{
		for (auto& patch : this->bezierPatchesData)
		{
			for (int i = 0; i < BezierPatchData::CONTROL_POINTS_PER_EDGE; i++)
			{
				if (auto point = patch.controlPoints[i][j])
				{
					result["controlPoints"].push_back(json::object({ { "id", point->GetShapeId() } }));
				}
			}
		}
	}
	result["size"] = json::object({
	{"u", u_patches * 4},
	{"v", v_patches * 4},
		});
	result["samples"] = json::object({
		{"u", u_subdivisions},
		{"v", v_subdivisions},
		});

	return result;
}
