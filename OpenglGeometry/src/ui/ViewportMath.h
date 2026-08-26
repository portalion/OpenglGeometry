#pragma once
#include <optional>
#include <imgui/imgui.h>
#include "core/Base.h"
#include "Algebra.h"

class Scene;

namespace GUI
{
	struct ViewportCamera
	{
		Algebra::Matrix4 view;
		Algebra::Matrix4 projection;
		Algebra::Vector4 position;
		Algebra::Vector4 right;
		Algebra::Vector4 up;
		Algebra::Vector4 forward;
	};

	std::optional<ViewportCamera> ActiveViewportCamera(Ref<Scene> scene);

	bool ProjectToViewport(const ViewportCamera& camera, const Algebra::Vector4& world,
		const ImVec2& rectMin, const ImVec2& rectMax, ImVec2& outScreen);.
	Algebra::Vector4 ViewportRayDirection(const ViewportCamera& camera, const ImVec2& screen,
		const ImVec2& rectMin, const ImVec2& rectMax);
}
