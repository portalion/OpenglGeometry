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
	SetScale({ 1.f, 1.f, 1.f });

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

BezierSurface::BezierSurface(std::vector<std::shared_ptr<Point>> controlPoints, unsigned int uSize, unsigned int vSize, ShapeList* list)
	:shapeList{ list }
{
	renderingMode = RenderingMode::PATCHES;
	u_patches = (uSize - 1) / 3;
	v_patches = (vSize - 1) / 3;

	std::vector<std::vector<std::shared_ptr<Point>>> controlPoints2D;

	int row = 0;
	int column = 0;
	for (int i = 0; i < vSize; i++)
	{
		column = 0;
		controlPoints2D.push_back(std::vector<std::shared_ptr<Point>>());
		for (int j = 0; j < uSize; j++)
		{
			auto point = controlPoints[i * uSize + j];
			point->Attach(this);
			controlPoints2D[row].push_back(point);

			if (column % 4 == 3 && j != uSize - 1)
				j--;

			column++;
		}
		if (row % 4 == 3 && i != vSize - 1)
			i--;
		row++;
	}

	for (int i = 0; i < v_patches; i++)
	{
		for (int j = 0; j < u_patches; j++)
		{
			BezierPatchData patch;
			for (int x = 0; x < BezierPatchData::CONTROL_POINTS_PER_EDGE; x++)
			{
				for (int y = 0; y < BezierPatchData::CONTROL_POINTS_PER_EDGE; y++)
				{
					patch.controlPoints[x][y] = controlPoints2D[i * 4 + x][j * 4 + y];
				}
			}
			bezierPatchesData.push_back(patch);
		}
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

std::shared_ptr<BezierSurface> BezierSurface::Deserialize(const json& j, ShapeList* list)
{
	auto id = j["id"].get<unsigned int>();
	auto name = j["name"].get<std::string>();
	auto uSize = j["size"]["u"].get<int>();
	auto vSize = j["size"]["v"].get<int>();
	auto u_subdivisions = j["samples"]["u"].get<int>();
	auto v_subdivisions = j["samples"]["v"].get<int>();
	std::vector<std::shared_ptr<Point>> controlPoints;
	for (auto pt : j["controlPoints"])
	{
		auto point = list->GetPointWithId(pt["id"].get<unsigned int>());
		if (point)
		{
			point->removable = false;
			controlPoints.push_back(point);
		}
	}

	auto result = std::make_shared<BezierSurface>(controlPoints, uSize, vSize, list);
	result->InitName(id, name);
	result->u_subdivisions = u_subdivisions;
	result->v_subdivisions = v_subdivisions;

	return result;
}

std::vector<std::shared_ptr<Point>> ReconstructFlatControlPointsFromPatches(
	const std::vector<BezierPatchData>& bezierPatchesData,
	int u_patches, int v_patches)
{
	constexpr int PATCH_SIZE = BezierPatchData::CONTROL_POINTS_PER_EDGE;
	constexpr int STRIDE = PATCH_SIZE - 1; // For C0 continuity, stride is 3

	int uSize = u_patches * STRIDE + 1;
	int vSize = v_patches * STRIDE + 1;

	// Step 1: reconstruct 2D grid with deduplication
	std::vector<std::vector<std::shared_ptr<Point>>> controlPoints2D(
		vSize, std::vector<std::shared_ptr<Point>>(uSize, nullptr));

	for (int i = 0; i < v_patches; ++i)
	{
		for (int j = 0; j < u_patches; ++j)
		{
			const auto& patch = bezierPatchesData[i * u_patches + j];
			for (int x = 0; x < PATCH_SIZE; ++x)
			{
				for (int y = 0; y < PATCH_SIZE; ++y)
				{
					int row = i * STRIDE + x;
					int col = j * STRIDE + y;

					// Only assign if not already set
					if (!controlPoints2D[row][col])
					{
						controlPoints2D[row][col] = patch.controlPoints[x][y];
					}
				}
			}
		}
	}

	// Step 2: flatten the 2D vector to 1D row-major
	std::vector<std::shared_ptr<Point>> controlPoints;
	controlPoints.reserve(vSize * uSize);

	for (int i = 0; i < vSize; ++i)
	{
		for (int j = 0; j < uSize; ++j)
		{
			controlPoints.push_back(controlPoints2D[i][j]);
		}
	}

	return controlPoints;
}

json BezierSurface::Serialize() const
{
	json result;
	result["objectType"] = "bezierSurfaceC0";
	result["id"] = id;
	result["name"] = name;
	result["controlPoints"] = json::array();

	auto points = ReconstructFlatControlPointsFromPatches(
		bezierPatchesData, u_patches, v_patches);

	for (auto point : points)
	{
		result["controlPoints"].push_back(json::object({ { "id", point->GetShapeId() } }));
	}

	result["size"] = json::object({
	{"u", u_patches * 3 + 1},
	{"v", v_patches * 3 + 1},
		});
	result["samples"] = json::object({
		{"u", u_subdivisions},
		{"v", v_subdivisions},
		});

	return result;
}
