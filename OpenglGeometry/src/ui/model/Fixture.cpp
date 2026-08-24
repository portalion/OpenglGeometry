#include "Fixture.h"

#include <string>

namespace
{
	ObjectRow MakeRow(uint32_t id, std::string name, ObjectType type)
	{
		ObjectRow row;
		row.id = id;
		row.name = std::move(name);
		row.type = type;

		return row;
	}

	void Select(UiState& state, uint32_t id)
	{
		for (ObjectRow& row : state.objects)
		{
			if (row.id == id)
			{
				row.selected = true;
			}
		}
	}
}

UiState GUI::MakeFixture()
{
	UiState state;

	uint32_t nextId = 1;

	for (uint32_t i = 1; i <= 170; i++)
	{
		state.objects.push_back(MakeRow(nextId++, "Point " + std::to_string(i), ObjectType::Point));
	}

	state.objects[6].name = "";
	state.objects[41].name = "Torus 2";
	state.objects[99].name = "Point with a deliberately very long name that no panel is wide enough for";

	state.objects.push_back(MakeRow(nextId++, "Torus 1", ObjectType::Torus));
	state.objects.push_back(MakeRow(nextId++, "Torus 2", ObjectType::Torus));
	state.objects.push_back(MakeRow(nextId++, "Chain 1", ObjectType::Chain));
	state.objects.push_back(MakeRow(nextId++, "Bezier C0 1", ObjectType::BezierC0));
	state.objects.push_back(MakeRow(nextId++, "Wing spline", ObjectType::BezierC2));
	state.objects.push_back(MakeRow(nextId++, "Interpolated 1", ObjectType::InterpolatedC2));
	state.objects.push_back(MakeRow(nextId++, "Fuselage patch", ObjectType::BezierSurfaceC0));
	state.objects.push_back(MakeRow(nextId++, "Surface C2 1", ObjectType::BezierSurfaceC2));

	Select(state, 172);
	Select(state, 175);  
	Select(state, 177);  

	state.activeCurveId = 175;

	state.cursor.world = Algebra::Vector4(1.204f, 0.f, -0.335f, 1.f);
	state.cursor.screenX = 612.f;
	state.cursor.screenY = 348.f;
	state.cursor.selectionCentre = Algebra::Vector4(1.204f, 0.f, -0.335f, 1.f);

	state.pivot = PivotMode::Cursor;

	state.transform = TransformValues{};

	TorusValues torus;
	torus.largeRadius = 1.f;
	torus.tubeRadius = 0.2f;
	torus.samplesU = 16;
	torus.samplesV = 32;
	state.torus = torus;

	CurveValues curve;
	curve.controlPoints =
	{
		MakeRow(21, "Point 21", ObjectType::Point),
		MakeRow(22, "Point 22", ObjectType::Point),
		MakeRow(23, "Tip anchor", ObjectType::Point),
		MakeRow(24, "Point 24", ObjectType::Point),
		MakeRow(25, "Point 25", ObjectType::Point),
		MakeRow(26, "Point 26", ObjectType::Point),
	};
	state.curve = curve;

	SurfaceValues surface;
	surface.sizeU = 7;
	surface.sizeV = 4;
	state.surface = surface;

	state.statusMessage = "fixture data - nothing here is connected to a scene";

	return state;
}
