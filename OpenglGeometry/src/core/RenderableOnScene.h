#pragma once
#include <string>
#include <managers/ShaderManager.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include "engine/Renderable.h"

//TODO: ADD ID MANAGER
//TODO: Move IMGUI / Menus / ID into composition?
class RenderableOnScene : public Renderable<PositionVertexData>
{
private:
	unsigned int id;
	
protected:
	std::string name;

	bool somethingChanged = false;
	virtual RenderableMesh<PositionVertexData> GenerateMesh() = 0;
	virtual std::string GetTypeName() const = 0;
	virtual bool DisplayParameters() = 0;
public:
	RenderableOnScene();
	virtual ~RenderableOnScene() = default;
	void InitName();
	
	void DisplayMenu();
	virtual void Update();

	inline std::string GetName() const { return name; }
	inline ImGuiID GetId() const { return ImGui::GetID(name.c_str()); }

	std::string GenerateLabelWithId(std::string label);
};