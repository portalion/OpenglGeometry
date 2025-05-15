#pragma once
#include "core/RenderableOnScene.h"
#include "objects/Point.h"
#include <vector>
#include "Polyline.h"
#include <UI/SelectedShapes.h>
#include "App.h"
#include "core/Globals.h"

class BezierCurve : public RenderableOnScene, public IObserver
{
private:
	bool displayPolyline = false;
	std::vector<std::weak_ptr<Point>> points;

	inline std::string GetTypeName() const override { return "Bezier Curve C0"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;

	Polyline polyline;
	bool HelperButton(ImGuiDir direction);
	SelectedShapes* selectedShapes = nullptr;
public:
	BezierCurve(std::vector<std::shared_ptr<Point>> points, SelectedShapes* shapes);
	~BezierCurve()
	{
		for (auto& point : points)
		{
			if (auto ptr = point.lock())
			{
				ptr->Detach(this);
			}
		}
	}
	inline void AddPoint(std::shared_ptr<Point> point)
	{
		point->Attach(this);
		somethingChanged = true;
		polyline.AddPoint(point);
		points.push_back(point);
	}
	void Update(const std::string&) override
	{
		somethingChanged = true;
	}
	inline void RemovePoint(std::weak_ptr<Point> point)
	{
		somethingChanged = true;
		if (auto ptr = point.lock())
		{
			ptr->Detach(this);
		}
		points.erase(std::remove_if(points.begin(), points.end(),
			[&point](const std::weak_ptr<Point>& p) { return p.lock() == point.lock(); }), points.end());
	}
	inline void Render() const override
	{
		auto shader = ShaderManager::GetInstance().GetShader(AvailableShaders::Bezier);
		shader->Bind();
		shader->SetUniformMat4f("u_viewMatrix", App::camera.GetViewMatrix());
		shader->SetUniformMat4f("u_projectionMatrix", App::projectionMatrix);
		shader->SetUniformVec4f("u_cameraPos", App::camera.GetPosition());
		RenderableOnScene::Render();
		ShaderManager::GetInstance().GetShader(AvailableShaders::Default)->Bind();
		if (displayPolyline)
		{
			polyline.Render();
		}
	}
	
	void Update() override
	{
		RenderableOnScene::Update();
		polyline.Update();
	}
};

