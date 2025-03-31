#pragma once
#include <string>
#include <engine/Renderer.h>
#include <managers/ShaderManager.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include "Algebra.h"

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
	Algebra::Quaternion rotation;

	virtual std::string GetTypeName() const = 0;
	virtual RenderableOnSceneMesh GenerateMesh() = 0;
	virtual bool DisplayParameters() = 0;
public:
	RenderableOnScene();
	virtual ~RenderableOnScene() = default;
	
	inline void SetPosition(Algebra::Vector4 pos) { position = pos; }
	inline void SetRotation(Algebra::Quaternion rot) { rotation = rot; }
	inline Algebra::Matrix4 GetModelMatrix() { return Algebra::Matrix4::Translation(position) * rotation.ToMatrix(); };
	inline Algebra::Vector4 GetPosition() { return position; };
	inline Algebra::Quaternion GetRotation() { return rotation.Normalize(); }

	void Move(Algebra::Vector4 translation);
	void Rotate(Algebra::Quaternion rotation);
	void InitName();
	void Update();
	void DisplayMenu();
	void Render() const;

	inline std::string GetName() const { return name; }
	inline ImGuiID GetId() const { return ImGui::GetID(name.c_str()); }
};

