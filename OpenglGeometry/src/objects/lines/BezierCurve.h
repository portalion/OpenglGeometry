#pragma once
#include "core/RenderableOnScene.h"
#include "objects/Point.h"
#include <vector>
#include "Polyline.h"
#include <UI/SelectedShapes.h>
#include "App.h"

class BezierCurve : public RenderableOnScene, public IObserver, public ILine
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
	void Render() const override;
	
	inline void Update() override
	{
		RenderableOnScene::Update();
		polyline.Update();
	}

	static std::shared_ptr<BezierCurve> Deserialize(const json& j, ShapeList* list);
	json Serialize() const override;

	void ChangePoint(unsigned int idFrom, std::shared_ptr<Point> toPoint) override {}
};

