#pragma once
#include "core/RenderableOnScene.h"
#include "Point.h"
#include <vector>
#include "Polyline.h"
#include <UI/SelectedShapes.h>
#include "App.h"
#include "core/Globals.h"

class BezierCurveC2 : public RenderableOnScene, public IObserver
{
private:
	bool displayPolyline = false;
	bool displayBezierPoints = false;
	bool displayBezierPolyline = false;
	std::vector<std::weak_ptr<Point>> points;
	std::vector<std::shared_ptr<Point>> bezierPoints;

	inline std::string GetTypeName() const override { return "Bezier Curve C2"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	RenderableMesh<PositionVertexData> GenerateMeshFromBezier();
	bool DisplayParameters() override;

	Polyline polyline;
	Polyline bernsteinPolyline;
	bool HelperButton(ImGuiDir direction);
	SelectedShapes* selectedShapes = nullptr;
public:
	BezierCurveC2(std::vector<std::shared_ptr<Point>> points, SelectedShapes* shapes);
	~BezierCurveC2()
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
		auto defShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);
		defShader->Bind();
		if (displayPolyline)
		{
			polyline.Render();
		}
		if (displayBezierPoints)
		{
			defShader->SetUniformVec4f("u_color", Globals::defaultVirtualPointColor);
			for (const auto& pt : bezierPoints)
			{
				defShader->SetUniformMat4f("u_modelMatrix", pt->GetModelMatrix());
				pt->Render();
			}
			defShader->SetUniformVec4f("u_color", Globals::defaultPointsColor);
		}
		if (displayBezierPolyline)
		{
			defShader->SetUniformVec4f("u_color", Globals::defaultMiddlePointColor);
			defShader->SetUniformMat4f("u_modelMatrix", bernsteinPolyline.GetModelMatrix());
			bernsteinPolyline.Render();
			defShader->SetUniformVec4f("u_color", Globals::defaultPointsColor);
		}
	}

	void Update() override
	{
		RenderableOnScene::Update();
		polyline.Update();
		for (const auto& pt : bezierPoints)
		{
			pt->Update();
		}
		bernsteinPolyline.Update();
	}
};