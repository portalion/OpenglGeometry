#pragma once
#include "core/RenderableOnScene.h"
#include "UI/ShapeList.h"
#include "objects/Point.h"
#include "App.h"
#include "BezierSurface.h"

class BezierSurfaceC2 : public RenderableOnScene, public IObserver
{
private:
	int u_subdivisions = 8;
	int v_subdivisions = 8;
	bool drawBoorPolygon = false;
	bool drawBezierPolygon = false;
	int u_patches = 1;
	int v_patches = 1;

	std::vector<std::shared_ptr<Polyline>> boorPolygon;
	std::vector<std::shared_ptr<Polyline>> bezierPolygon;
	ShapeList* shapeList;
	std::vector<BezierPatchData> bezierPatchesData;
	std::vector<BezierPatchData> bezierPatchesDataForB;
	std::vector<std::shared_ptr<Point>> bernsteinPoints;
	inline std::string GetTypeName() const override { return "Bezier Surface C2"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	bool DisplayParameters() override;

	std::vector<std::vector<std::shared_ptr<Point>>> GetBernsteinGrids() const;

	void GeneratePlane(int xPatches = 1, int yPatches = 1, float sizeX = 10.f, float sizeY = 10.f);
	void GenerateCylinder(int radiusPatches = 1, int heightPatches = 1, float r = 10.f, float height = 10.f);
public:
	BezierSurfaceC2(ShapeList* shapeLis, bool isCylinder, float sizex, float sizey, int xpatch, int ypatcht);
	BezierSurfaceC2(std::vector<std::shared_ptr<Point>> controlPoints, unsigned int uSize, unsigned int vSize, ShapeList* list);
	~BezierSurfaceC2();

	void Render() const override;
	void Update(const std::string& message_from_subject) override;

	static std::shared_ptr<BezierSurfaceC2> Create(ShapeList* shapeList, bool isCylinder, float sizex, float sizey, int xpatch, int ypatch);

	static std::shared_ptr<BezierSurfaceC2> Deserialize(const json& j, ShapeList* list);
	json Serialize() const override;
};

