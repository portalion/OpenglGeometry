#pragma once
#include <string>
#include <engine/Renderer.h>
#include <managers/ShaderManager.h>
#include <imgui/imgui.h>

struct RenderableOnSceneMesh
{
	std::vector<PositionVertexData> vertices;
	std::vector<unsigned int> indices;
};

//TODO: ADD ID MANAGER
class RenderableOnScene
{
private:
	Renderer<PositionVertexData> renderer;
	bool somethingChanged = false;
	unsigned int id;
	
	void SaveMesh();
protected:
	std::string name;
	RenderingMode renderingMode;
	Algebra::Vector4 position;

	virtual std::string GetTypeName() const = 0;
	virtual RenderableOnSceneMesh GenerateMesh() = 0;
	virtual bool DisplayParameters() = 0;
public:
	RenderableOnScene();
	virtual ~RenderableOnScene() = default;
	
	inline Algebra::Matrix4 GetModelMatrix() { return Algebra::Matrix4::Translation(position); };
	void Move(Algebra::Vector4 translation);
	void InitName();
	void Update();
	void DisplayMenu();
	void Render() const;

	inline std::string GetName() const { return name; }
	inline ImGuiID GetId() const { return ImGui::GetID(name.c_str()); }
};

