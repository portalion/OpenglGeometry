#include "ShapeList.h"
#include "ShapeCreator.h"
#include <objects/lines/BezierCurve.h>
#include <objects/lines/InterpolatedBezierCurve.h>
#include <objects/lines/BezierCurveC2.h>
#include <objects/lines/ILine.h>

void ShapeList::CreateShapeButton()
{
    auto availableShapes = ShapeCreator::GetShapeList();
    static int currentShapeIndex = 0;

    if (ImGui::Button("Create shape"))
    {
        auto newShape = shapeCreator.CreateShape(availableShapes[currentShapeIndex].first, this);

		if (!newShape)
		{
			return;
		}

        shapes.push_back(newShape);

        if (availableShapes[currentShapeIndex].first == ShapeEnum::Point)
        {
            auto lines = selectedShapes->GetSelectedWithType<ILine>();
            for (auto line : lines)
            {
                line->AddPoint(std::dynamic_pointer_cast<Point>(newShape));
            }
        }
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(Globals::rightInterfaceWidth / 3.f);
    if (ImGui::BeginCombo("##shape", availableShapes[currentShapeIndex].second.c_str()))
    {
        for (int i = 0; i < availableShapes.size(); ++i)
        {
            const bool isSelected = (currentShapeIndex == i);
            if (ImGui::Selectable(availableShapes[i].second.c_str(), isSelected))
            {
                currentShapeIndex = i;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

ShapeList::ShapeList(AxisCursor* axis, SelectedShapes* selectedShapes)
    :selectedShapes{selectedShapes}, shapeCreator{selectedShapes, axis}
{
    defaultShader = ShaderManager::GetInstance().GetShader(AvailableShaders::Default);
}

std::shared_ptr<Point> ShapeList::GetPointByPosition(Algebra::Matrix4 VP, Algebra::Vector4 position) const
{
    const float similarityThreshold = 0.02f;
    Algebra::Vector4 worldPos(0.f, 0.f, 0.f, 1.f);

    for (const auto& shape : shapes)
    {
        if (auto point = std::dynamic_pointer_cast<Point>(shape))
        {
            Algebra::Matrix4 MVP = VP * point->GetModelMatrix();
            Algebra::Vector4 clipPos = MVP * worldPos;

            clipPos.z = 0.f;

            if (clipPos.w != 0.f)
            {
                clipPos = clipPos / clipPos.w;
            }

            if (std::abs(position.x - clipPos.x) < similarityThreshold &&
                std::abs(position.y - clipPos.y) < similarityThreshold)
            {
                return point;
            }
        }
    }
    return nullptr;
}

void ShapeList::DisplayUI()
{
    if (ImGui::CollapsingHeader("Shape List", ImGuiTreeNodeFlags_Leaf))
    {
        ImGui::BeginChild("ShapeList", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);

        if (shapes.empty())
        {
            ImGui::Text("No shapes available.");
        }
        else
        {
            bool ctrlPressed = ImGui::GetIO().KeyCtrl;

            for (const auto& renderable : shapes)
            {
                if (ImGui::Selectable(renderable->GenerateLabelWithId(renderable->GetName()).c_str(), 
                    selectedShapes->IsSelected(renderable)))
                {
                    if (ctrlPressed)
                    {
                        selectedShapes->ToggleShape(renderable);
                    }
                    else
                    {
                        selectedShapes->Clear();
                        selectedShapes->AddShape(renderable);
                    }
                }
            }
        }
        ImGui::EndChild();
        
        CreateShapeButton();
       
        ImGui::SameLine();

        ImGui::BeginDisabled(selectedShapes->IsEmpty());
        if (ImGui::Button("Remove shapes"))
        {
            shapes.erase(
                std::remove_if(shapes.begin(), shapes.end(),
                    [&](const std::shared_ptr<RenderableOnScene>& shape) {
                        if (const auto& p = std::dynamic_pointer_cast<Point>(shape))
                        {
                            if (!p->removable) return false;
                        }
                        bool shouldRemove = selectedShapes->IsSelected(shape);
                        return shouldRemove;
                    }),
                shapes.end()
            );
            selectedShapes->Clear();
        }
        ImGui::EndDisabled();
    }
}

void ShapeList::Update()
{
    for (auto& renderable : shapes)
    {
        renderable->Update();
    }
}

void ShapeList::Render()
{
    for (auto& renderable : shapes)
    {
        defaultShader->SetUniformMat4f("u_modelMatrix", renderable->GetModelMatrix());
        renderable->Render();
    }
}

void ShapeList::AddPoint(std::shared_ptr<Point> point)
{
    shapes.push_back(point);
    point->removable = false;
}

void ShapeList::RemovePoint(std::shared_ptr<Point> point)
{
    point->removable = true;
}
