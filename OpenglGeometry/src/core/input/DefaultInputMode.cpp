#include "DefaultInputMode.h"

DefaultInputMode::DefaultInputMode(Window* window, Camera* camera)
    :InputMode(window, camera)
{
}

void DefaultInputMode::HandleInput(const std::vector<std::shared_ptr<RenderableOnScene>>& selectedItems)
{
    if (ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
    {
        return;
    }

    camera->HandleInput();

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        auto mousePos = ImGui::GetMousePos();
        draggingPoint = GetMousePoint(mousePos.x, mousePos.y).Normalize();
        return;
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        auto mousePos = ImGui::GetMousePos();
        Algebra::Vector4 q = GetMousePoint(mousePos.x, mousePos.y).Normalize();
        if (q == draggingPoint)
        {
            return;
        }
        float theta = acosf(draggingPoint * q);
        auto w = q.Cross(draggingPoint).Normalize();
        auto tempMat = Algebra::Matrix4(0, 0, 0, 0);
        tempMat[1][0] = w.z;
        tempMat[0][1] = -w.z;
        tempMat[0][2] = -w.y;
        tempMat[2][0] = w.y;
        tempMat[2][1] = w.x;
        tempMat[1][2] = -w.x;
        auto rotation = Algebra::Matrix4::Identity() + sinf(theta) * tempMat + ((1.f - cosf(theta)) * tempMat * tempMat);
        draggingPoint = q;
    }
}

Algebra::Vector4 DefaultInputMode::GetMousePoint(float x, float y)
{
    float screenWidth = static_cast<float>(window->GetWidth());
    float screenHeight = static_cast<float>(window->GetHeight());
    float scale = fminf(screenHeight, screenWidth) - 1.f;

    x = (2.f * x - screenWidth + 1.f) / scale;
    y = (2.f * y - screenHeight + 1.f) / -scale;

    float z = 0;
    float d = x * x + y * y;
    if (d <= 1.f / 2.f)
    {
        z = sqrtf(1 - d);
    }
    else
    {
        z = 1.f / 2.f / sqrtf(d);
    }

    return Algebra::Vector4(x, y, z, 0);
}