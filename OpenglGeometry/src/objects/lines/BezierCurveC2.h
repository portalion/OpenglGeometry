#pragma once
#include "core/RenderableOnScene.h"
#include "objects/Point.h"
#include <vector>
#include "Polyline.h"
#include <UI/SelectedShapes.h>
#include "App.h"
#include "core/Globals.h"

class BernsteinObserver : public IObserver
{
public:
	bool* somethingChanged;
	int* moved;
	inline void Update(const std::string& v) override
	{
		int idx;
		try {
			idx =  std::stoi(v);
		}
		catch (const std::invalid_argument& e) {
			// v is not a number
			idx = -1;
		}
		catch (const std::out_of_range& e) {
			// number out of int range
			idx = -1;
		}
		if (idx == -1)
			return;

		if (somethingChanged && moved)
		{
			*somethingChanged = true;
			*moved = idx;
		}
	}
};

class BezierCurveC2 : public RenderableOnScene, public IObserver, public ILine
{
private:
	bool refreshBernstein = false;
	int movedIndex = -1;

	bool displayPolyline = false;
	bool displayBezierPoints = false;
	bool displayBezierPolyline = false;
	std::vector<std::weak_ptr<Point>> points;
	std::vector<std::shared_ptr<BernPoint>> bezierPoints;

	inline std::string GetTypeName() const override { return "Bezier Curve C2"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;

	Polyline polyline;
	Polyline bernsteinPolyline;
	bool HelperButton(ImGuiDir direction);
	SelectedShapes* selectedShapes = nullptr;
	BernsteinObserver observer;
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
		for (auto& point : bezierPoints)
		{
			point->Detach(&observer);
		}
	}
	inline void AddPoint(std::shared_ptr<Point> point)
	{
		point->Attach(this);
		somethingChanged = true;
		polyline.AddPoint(point);
		points.push_back(point);
		if (points.size() >= 4)
		{
			for (int i = 0; i < 4; i++)
			{
				bezierPoints.push_back(std::make_shared<BernPoint>(bezierPoints.size()));
				bernsteinPolyline.AddPoint(bezierPoints.back());
				bezierPoints.back()->Attach(&observer);
			}
		}
	}
	inline void Update(const std::string&) override
	{
		somethingChanged = true;
		for (auto it = points.begin(); it != points.end();)
		{
			if (auto point = it->lock())
			{
				++it;
			}
			else
			{
				it = RemovePoint(it);
			}
		}
	}

	inline std::vector<std::weak_ptr<Point>>::iterator RemovePoint(std::vector<std::weak_ptr<Point>>::iterator it)
	{
		somethingChanged = true;
		if (auto ptr = it->lock())
		{
			ptr->Detach(this);
		}
		polyline.RemovePoint(*it);
		auto result = points.erase(it);
		if (points.size() >= 3)
		{
			for (int i = 0; i < 4; i++)
			{
				bernsteinPolyline.RemovePoint(bezierPoints.back());
				bezierPoints.pop_back();
			}
		}
		return result;
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

		if (points.size() >= 4)
		{
			for (int i = 0; i < 4; i++)
			{
				bernsteinPolyline.RemovePoint(bezierPoints.back());
				bezierPoints.pop_back();
			}
		}
	}

	inline void Render() const override
	{
		auto shader = ShaderManager::GetInstance().GetShader(AvailableShaders::BezierLine);
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

	void GetClickedPoint();
	void Update() override;
	void UpdateBasedOnDeBoor();
	void UpdateBasedOnBernstein(int movedBezierIndex, Algebra::Vector4 moved);

	json Serialize() const override;
};