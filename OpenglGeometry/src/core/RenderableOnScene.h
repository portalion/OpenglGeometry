#pragma once
#include <string>
#include <engine/Renderer.h>
#include <managers/ShaderManager.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include "Transformable.h"

struct RenderableOnSceneMesh
{
	std::vector<PositionVertexData> vertices;
	std::vector<unsigned int> indices;
};

//TODO: ADD ID MANAGER
//TODO: Change into smaller classes: Renderable, TransformableRenderable, RenderableDependentOnObserver, ObservableRenderable, RenderableWithOwnInputOnSelection
class RenderableOnScene : public Transformable
{
private:
	Renderer<PositionVertexData> renderer;
	bool somethingChanged = false;
	unsigned int id;
	
	void SaveMesh();
protected:
	std::string name;
	RenderingMode renderingMode;

	virtual std::string GetTypeName() const = 0;
	virtual RenderableOnSceneMesh GenerateMesh() = 0;
	virtual bool DisplayParameters() = 0;
public:
	RenderableOnScene();
	virtual ~RenderableOnScene() = default;
	
	void InitName();
	void Update();
	void DisplayMenu();
	void Render() const;

	inline std::string GetName() const { return name; }
	inline ImGuiID GetId() const { return ImGui::GetID(name.c_str()); }

	std::string GenerateLabelWithId(std::string label);
};