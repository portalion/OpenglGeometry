#pragma once
#include "core/RenderableOnScene.h"
#include "UI/ShapeList.h"
#include "objects/Point.h"
#include "App.h"
#include <objects/lines/Polyline.h>

struct BezierPatchData
{
	static const int CONTROL_POINTS_PER_EDGE = 4;
	std::shared_ptr<Point> controlPoints[CONTROL_POINTS_PER_EDGE][CONTROL_POINTS_PER_EDGE];
};

class BezierSurface : public RenderableOnScene, public IObserver
{
private:
	int u_subdivisions = 8;
	int v_subdivisions = 8;
	bool drawBezierPolygon = false;
	int u_patches = 1;
	int v_patches = 1;
	std::vector<std::shared_ptr<Polyline>> bezierPolygon;
	ShapeList* shapeList;
	std::vector<BezierPatchData> bezierPatchesData;
	inline std::string GetTypeName() const override { return "Bezier Surface"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;

	void GeneratePlane(int xPatches = 1, int yPatches = 1, float sizeX = 10.f, float sizeY = 10.f);
	void GenerateCylinder(int radiusPatches = 1, int heightPatches = 1, float r = 10.f, float height = 10.f);
public:
	BezierSurface(ShapeList* shapeList, bool isCylinder, float sizex, float sizey, int xpatch, int ypatch);
	BezierSurface(std::vector<std::shared_ptr<Point>> controlPoints, unsigned int uSize, unsigned int vSize, ShapeList* list);
	~BezierSurface();

	void Render() const override;
	void Update(const std::string& message_from_subject) override;

	static std::shared_ptr<BezierSurface> Create(ShapeList* shapeList, bool isCylinder, float sizex, float sizey, int xpatch, int ypatch);

	static std::shared_ptr<BezierSurface> Deserialize(const json& j, ShapeList* list);
	json Serialize() const override;
};

