#pragma once
#include "core/RenderableOnScene.h"
#include "UI/ShapeList.h"
#include "objects/Point.h"
#include "App.h"

struct BezierPatchData
{
	static const int CONTROL_POINTS_PER_EDGE = 4;
	std::shared_ptr<Point> controlPoints[CONTROL_POINTS_PER_EDGE][CONTROL_POINTS_PER_EDGE];
};

class BezierSurface : public RenderableOnScene, public IObserver
{
private:
	ShapeList* shapeList;
	std::vector<BezierPatchData> bezierPatchesData;
	inline std::string GetTypeName() const override { return "Bezier Surface"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;

	void GeneratePlane(int xPatches = 1, int yPatches = 1, float sizeX = 10.f, float sizeY = 10.f);
	void GenerateCylinder();
public:
	BezierSurface(ShapeList* shapeList);
	~BezierSurface();

	inline void Render() const override
	{
		auto shader = ShaderManager::GetInstance().GetShader(AvailableShaders::BezierSurface);
		shader->Bind();
		shader->SetUniformMat4f("u_viewMatrix", App::camera.GetViewMatrix());
		shader->SetUniformMat4f("u_projectionMatrix", App::projectionMatrix);
		RenderableOnScene::Render();

		auto defShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);
		defShader->Bind();
		for (auto& patch : bezierPatchesData)
		{
			for (int i = 0; i < patch.CONTROL_POINTS_PER_EDGE; i++)
			{
				for (int j = 0; j < patch.CONTROL_POINTS_PER_EDGE; j++)
				{
					auto& pt = patch.controlPoints[i][j];
					defShader->SetUniformMat4f("u_modelMatrix", pt->GetModelMatrix());
					pt->Render();
				}
			}
		}

	}

	void Update(const std::string& message_from_subject) override;
};

