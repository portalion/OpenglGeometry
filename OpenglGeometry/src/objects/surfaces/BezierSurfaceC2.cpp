#include "BezierSurfaceC2.h"
#include <array>

RenderableMesh<PositionVertexData> BezierSurfaceC2::GenerateMesh()
{
	RenderableMesh<PositionVertexData> result;

	static const float A[4][4] = {
			{ 1.f / 6, 4.f / 6.f, 1.f / 6.f, 0.f },
			{ 0.f,     4.f / 6.f, 2.f / 6.f, 0.f },
			{ 0.f,     2.f / 6.f, 4.f / 6.f, 0.f },
			{ 0.f,     1.f / 6.f, 4.f / 6.f, 1.f / 6.f }
	};

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
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				Algebra::Vector4 b = Algebra::Vector4();
				for (int k = 0; k < 4; ++k)
					b += Q[i][k] * A[j][k];
				bernsteinPoints[patchIndex * 16 + i * 4 + j]->SetPosition(b);
			}
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
	SetScale(1.f);

	ImGui::DragInt(GenerateLabelWithId("u subdivision").c_str(), &u_subdivisions, 1, 2, 50);
	ImGui::DragInt(GenerateLabelWithId("v subdivision").c_str(), &v_subdivisions, 1, 2, 50);

	return false;
}

void BezierSurfaceC2::GeneratePlane(int xPatches, int yPatches, float sizeX, float sizeY)
{
	const int columns = xPatches + 3;
	const int rows = yPatches + 3;

	float dx = sizeX / static_cast<float>(columns - 1);
	float dy = sizeY / static_cast<float>(rows - 1);

	Algebra::Vector4 startingPosition = Algebra::Vector4(sizeX / 2.f, sizeY / 2.f, 0.f);
	std::vector<std::shared_ptr<Point>> controlPoints;
	controlPoints.reserve(rows * columns);

	// Width is along X axis and height is along Y axis, Z axis is flat
	// storing points by columns then rows, like in matrix
	// 0 1 2 3
	// 4 5 6 7

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

	Algebra::Vector4 startingPosition = Algebra::Vector4(0.f, 0.f, height / 2.f);
	std::vector<std::shared_ptr<Point>> controlPoints;
	controlPoints.reserve(rows * columns);

	// Width is along X axis and height is along Y axis, Z axis is flat
	// storing points by columns then rows, like in matrix
	// 0 1 2 3
	// 4 5 6 7

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

BezierSurfaceC2::BezierSurfaceC2(ShapeList* shapeList)
	:shapeList{ shapeList }
{
	renderingMode = RenderingMode::PATCHES;
	GeneratePlane(3, 5, 30.f, 30.f);
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

void BezierSurfaceC2::Update(const std::string& message_from_subject)
{
	somethingChanged = true;
}

std::shared_ptr<BezierSurfaceC2> BezierSurfaceC2::Create(ShapeList* shapeList)
{
	return std::make_shared<BezierSurfaceC2>(shapeList);
}
