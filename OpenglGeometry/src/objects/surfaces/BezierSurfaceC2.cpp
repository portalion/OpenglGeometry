#include "BezierSurfaceC2.h"
#include <array>
#include "core/Globals.h"

void AddPolygonsc2(std::vector<std::shared_ptr<Polyline>>& polygons, const BezierPatchData& patch)
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

RenderableMesh<PositionVertexData> BezierSurfaceC2::GenerateMesh()
{
	RenderableMesh<PositionVertexData> result;

	static const float A[4][4] = {
			{ 1.f / 6, 4.f / 6.f, 1.f / 6.f, 0.f },
			{ 0.f,     4.f / 6.f, 2.f / 6.f, 0.f },
			{ 0.f,     2.f / 6.f, 4.f / 6.f, 0.f },
			{ 0.f,     1.f / 6.f, 4.f / 6.f, 1.f / 6.f }
	};
	bezierPatchesDataForB.clear();
	for (int patchIndex = 0; patchIndex < bezierPatchesData.size(); ++patchIndex)
	{
		auto patch = bezierPatchesData[patchIndex];

		std::array<std::array<Algebra::Vector4, 4>, 4> P;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				auto sp = patch.controlPoints[i][j];
				P[i][j] = sp->GetPosition();
			}

		std::array<std::array<Algebra::Vector4, 4>, 4> Q;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				Q[i][j] = Algebra::Vector4();
				for (int k = 0; k < 4; ++k)
					Q[i][j] += A[i][k] * P[k][j];
			}

		std::vector<Algebra::Vector4> B;
		B.reserve(16);

		BezierPatchData tmp;

		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				Algebra::Vector4 b = Algebra::Vector4();
				for (int k = 0; k < 4; ++k)
					b += Q[i][k] * A[j][k];
				bernsteinPoints[patchIndex * 16 + i * 4 + j]->SetPosition(b);
				tmp.controlPoints[i][j] = bernsteinPoints[patchIndex * 16 + i * 4 + j];
			}
		bezierPatchesDataForB.push_back(tmp);
	}
	bezierPolygon.clear();
	for (int i = 0; i < bezierPatchesDataForB.size(); i++)
	{
		AddPolygonsc2(bezierPolygon, bezierPatchesDataForB[i]);
	}

	for (auto pt : bernsteinPoints)
	{
		PositionVertexData vertex;
		vertex.Position = pt->GetPosition();
		vertex.Position.w = 1.f;
		result.vertices.push_back(vertex);
	}

	return result;
}

bool BezierSurfaceC2::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale({ 1.f, 1.f, 1.f });

	ImGui::DragInt(GenerateLabelWithId("u subdivision").c_str(), &u_subdivisions, 1, 2, 50);
	ImGui::DragInt(GenerateLabelWithId("v subdivision").c_str(), &v_subdivisions, 1, 2, 50);

	ImGui::Checkbox(GenerateLabelWithId("Draw Boor Polygon").c_str(), &drawBoorPolygon);
	ImGui::Checkbox(GenerateLabelWithId("Draw Bezier Polygon").c_str(), &drawBezierPolygon);

	return false;
}

void BezierSurfaceC2::GeneratePlane(int xPatches, int yPatches, float sizeX, float sizeY)
{
	const int columns = xPatches + 3;
	const int rows = yPatches + 3;

	float dx = sizeX / static_cast<float>(columns - 1);
	float dy = sizeY / static_cast<float>(rows - 1);

	Algebra::Vector4 startingPosition = Algebra::Vector4();
	std::vector<std::shared_ptr<Point>> controlPoints;
	controlPoints.reserve(rows * columns);

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < columns; ++j)
		{
			auto heightOffset = Algebra::Vector4(0.f, i * dy, 0.f);
			auto widthOffset = Algebra::Vector4(j * dx, 0.f, 0.f);

			auto pos = startingPosition + widthOffset + heightOffset;

			auto point = std::make_shared<Point>();
			point->InitName();
			point->SetPosition(startingPosition + heightOffset + widthOffset);
			point->Update();

			shapeList->AddPoint(point);
			point->Attach(this);
			controlPoints.push_back(point);
		}
	}

	for (int patchIndex = 0; patchIndex < xPatches * yPatches; ++patchIndex)
	{
		int startingI = patchIndex / xPatches;
		int startingJ = patchIndex % xPatches;
		BezierPatchData patch;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				int index = (startingI + i) * columns + (startingJ + j) % columns;
				patch.controlPoints[i][j] = controlPoints[index];
			}
		}

		bezierPatchesData.push_back(patch);
	}

	for (int i = 0; i < bezierPatchesData.size() * 16; ++i)
	{
		auto point = std::make_shared<Point>();
		point->SetPosition(Algebra::Vector4());
		bernsteinPoints.push_back(point);
	}

}

void BezierSurfaceC2::GenerateCylinder(int radiusPatches, int heightPatches, float radius, float height)
{
	const int columns = radiusPatches + 2;
	const int rows = heightPatches + 3;

	float dHeight = height / static_cast<float>(rows - 1);
	float dAngle = 2.f * std::numbers::pi_v<float> / static_cast<float>(columns);

	Algebra::Vector4 startingPosition = Algebra::Vector4();
	std::vector<std::shared_ptr<Point>> controlPoints;
	controlPoints.reserve(rows * columns);

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
			controlPoints.push_back(point);
		}
	}


	for (int patchIndex = 0; patchIndex < (radiusPatches + 3) * heightPatches; ++patchIndex)
	{
		BezierPatchData patch;
		int startingI = patchIndex / (radiusPatches + 3);
		int startingJ = patchIndex % (radiusPatches + 3);

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				int index = (startingI + i) * columns + (startingJ + j) % columns;
				patch.controlPoints[i][j] = controlPoints[index];
			}
		}

		bezierPatchesData.push_back(patch);
	}

	for (int i = 0; i < bezierPatchesData.size() * 16; ++i)
	{
		auto point = std::make_shared<Point>();
		point->SetPosition(Algebra::Vector4());
		bernsteinPoints.push_back(point);
	}
}

BezierSurfaceC2::BezierSurfaceC2(ShapeList* shapeList, bool isCylinder, float sizex, float sizey, int xpatch, int ypatch)
	:shapeList{ shapeList }
{
	renderingMode = RenderingMode::PATCHES;
	u_patches = xpatch;
	v_patches = ypatch;
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
		AddPolygonsc2(boorPolygon, bezierPatchesData[i]);
	}

}

BezierSurfaceC2::BezierSurfaceC2(std::vector<std::shared_ptr<Point>> controlPoints, unsigned int uSize, unsigned int vSize, ShapeList* list)
	:shapeList{ list }
{
	renderingMode = RenderingMode::PATCHES;
	auto xPatches = uSize - 3;
	auto yPatches = vSize - 3;

	for (int patchIndex = 0; patchIndex < xPatches * yPatches; ++patchIndex)
	{
		int startingI = patchIndex / xPatches;
		int startingJ = patchIndex % xPatches;
		BezierPatchData patch;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				int index = (startingI + i) * uSize + (startingJ + j) % uSize;
				patch.controlPoints[i][j] = controlPoints[index];
				controlPoints[index]->Attach(this);
			}
		}

		bezierPatchesData.push_back(patch);
	}

	for (int i = 0; i < bezierPatchesData.size() * 16; ++i)
	{
		auto point = std::make_shared<Point>();
		point->SetPosition(Algebra::Vector4());
		bernsteinPoints.push_back(point);
	}

	for (int i = 0; i < bezierPatchesData.size(); i++)
	{
		AddPolygonsc2(bezierPolygon, bezierPatchesData[i]);
	}
}

BezierSurfaceC2::~BezierSurfaceC2()
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

void BezierSurfaceC2::Render() const
{
	for (auto polygon : bezierPolygon)
		polygon->Update();
	for (auto polygon : boorPolygon)
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

	if (drawBezierPolygon)
	{
		for (auto polygon : bezierPolygon)
			polygon->Render();
	} 
	if (drawBoorPolygon)
	{
		for (auto polygon : boorPolygon)
			polygon->Render();
	}
}

void BezierSurfaceC2::Update(const std::string& message_from_subject)
{
	somethingChanged = true;
}

std::shared_ptr<BezierSurfaceC2> BezierSurfaceC2::Create(ShapeList* shapeList, bool isCylinder, float sizex, float sizey, int xpatch, int ypatch)
{
	return std::make_shared<BezierSurfaceC2>(shapeList, isCylinder, sizex, sizey, xpatch, ypatch);
}

std::shared_ptr<BezierSurfaceC2> BezierSurfaceC2::Deserialize(const json& j, ShapeList* list)
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

	auto result = std::make_shared<BezierSurfaceC2>(controlPoints, uSize, vSize, list);
	result->InitName(id, name);
	result->u_subdivisions = u_subdivisions;
	result->v_subdivisions = v_subdivisions;

	return result;
}

std::vector<std::shared_ptr<Point>> ReconstructDeBoorPointsFromC2Patches(
	const std::vector<BezierPatchData>& bezierPatchesData,
	unsigned int uSize,
	unsigned int vSize)
{
	std::vector<std::vector<std::shared_ptr<Point>>> controlPoints2D(
		vSize, std::vector<std::shared_ptr<Point>>(uSize, nullptr));

	int xPatches = uSize - 3;
	int yPatches = vSize - 3;

	for (int patchIndex = 0; patchIndex < xPatches * yPatches; ++patchIndex)
	{
		int startingI = patchIndex / xPatches;
		int startingJ = patchIndex % xPatches;

		const auto& patch = bezierPatchesData[patchIndex];

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				int row = startingI + i;
				int col = (startingJ + j) % uSize; // wraparound if needed

				if (!controlPoints2D[row][col])
				{
					controlPoints2D[row][col] = patch.controlPoints[i][j];
				}
			}
		}
	}

	// Flatten to 1D controlPoints vector (row-major)
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

json BezierSurfaceC2::Serialize() const
{
	json result;
	result["objectType"] = "bezierSurfaceC2";
	result["id"] = id;
	result["name"] = name;
	result["controlPoints"] = json::array();

	auto points = ReconstructDeBoorPointsFromC2Patches(
		bezierPatchesData, u_patches + 3, v_patches + 3);

	for (auto point : points)
	{
		result["controlPoints"].push_back(json::object({ { "id", point->GetShapeId() } }));
	}

	result["size"] = json::object({
		{"u", u_patches + 3},
		{"v", v_patches + 3},
		});
	result["samples"] = json::object({
		{"u", u_subdivisions},
		{"v", v_subdivisions},
		});

	return result;
}
