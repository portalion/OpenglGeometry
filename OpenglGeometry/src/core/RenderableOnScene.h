#pragma once
#include <string>
#include <managers/ShaderManager.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include "Transformable.h"
#include "engine/Renderable.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

//TODO: ADD ID MANAGER
//TODO: Move IMGUI / Menus / ID into composition?
class RenderableOnScene : public Transformable, public Renderable<PositionVertexData>
{
protected:
	unsigned int id;
	std::string name;

	bool somethingChanged = false;
	virtual RenderableMesh<PositionVertexData> GenerateMesh() = 0;
	virtual std::string GetTypeName() const = 0;
	virtual bool DisplayParameters() = 0;
public:
	RenderableOnScene();
	virtual ~RenderableOnScene() = default;
	void InitName();
	void InitName(unsigned int id, std::string name);
	
	void DisplayMenu();
	virtual void Update();

	inline std::string GetName() const { return name; }
	inline ImGuiID GetId() const { return ImGui::GetID(name.c_str()); }
	inline unsigned int GetShapeId() const { return id; }

	std::string GenerateLabelWithId(std::string label);

	virtual json Serialize() const = 0;
};