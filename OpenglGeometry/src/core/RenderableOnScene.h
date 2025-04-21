#pragma once
#include <string>
#include <managers/ShaderManager.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include "engine/Renderable.h"
#include "core/transformations/IExposesTransformationComponents.h"
#include "core/transformations/EmptyTransformationComponents.h"

//TODO: ADD ID MANAGER
//TODO: Move IMGUI / Menus / ID into composition?
class RenderableOnScene : public Renderable<PositionVertexData>, public IExposesTransformationComponents
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
	virtual Algebra::Matrix4 GetModelMatrix() override;
	virtual inline IPositionComponent* GetPositionComponent() override { return EmptyPositionComponent::GetInstance(); }
	virtual inline IRotationComponent* GetRotationComponent() override { return EmptyRotationComponent::GetInstance(); }
	virtual inline IScaleComponent* GetScaleComponent() override { return EmptyScaleComponent::GetInstance(); }
};