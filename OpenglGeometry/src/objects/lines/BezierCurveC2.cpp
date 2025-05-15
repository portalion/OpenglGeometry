#include "BezierCurveC2.h"
#include <algorithm>
#include "App.h"
#include "core/Globals.h"

RenderableMesh<PositionVertexData> BezierCurveC2::GenerateMesh()
{
	if (refreshBernstein)
	{
		if (movedIndex < bezierPoints.size() && movedIndex >= 0)
			UpdateBasedOnBernstein(movedIndex, bezierPoints[movedIndex]->GetMoved());
			
		refreshBernstein = false;
	}
	UpdateBasedOnDeBoor();

	RenderableMesh<PositionVertexData> mesh;
	if (bezierPoints.empty())
	{
		return {};
	}

	for (const auto& ptr : bezierPoints)
	{
		PositionVertexData vertex;
		vertex.Position = ptr->GetPosition();
		vertex.Position.w = 1.f;
		mesh.vertices.push_back(vertex);
	}

	return mesh;
}

bool BezierCurveC2::HelperButton(ImGuiDir direction)
{
	ImGui::SameLine();
	return ImGui::ArrowButton(GenerateLabelWithId(std::to_string(static_cast<int>(direction))).c_str(), direction);
}

bool BezierCurveC2::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale(1.f);

	ImGui::Checkbox(this->GenerateLabelWithId("Draw Polyline").c_str(), &displayPolyline);
	ImGui::Checkbox(this->GenerateLabelWithId("Draw Bernstein Points").c_str(), &displayBezierPoints);
	ImGui::Checkbox(this->GenerateLabelWithId("Draw Bernstein Polyline").c_str(), &displayBezierPolyline);
	if (ImGui::Button(GenerateLabelWithId("Add Selected Points").c_str()))
	{
		for (const auto& selected : selectedShapes->GetSelectedWithType<Point>())
		{
			if (!selected)
				continue;

			bool alreadyExists = false;
			for (const auto& weak : points)
			{
				if (auto existing = weak.lock())
				{
					if (existing == selected)
					{
						alreadyExists = true;
						break;
					}
				}
			}

			if (!alreadyExists)
			{
				AddPoint(selected);
			}
		}
		somethingChanged = true;
	}
	ImGui::Text("Control Points (%zu)", points.size());

	int index = 0;
	for (auto it = points.begin(); it != points.end();)
	{
		if (auto point = it->lock())
		{
			ImGui::PushID(index);

			ImGui::Text(point->GetName().c_str());
			ImGui::SameLine();
			if (ImGui::Button(GenerateLabelWithId("Remove").c_str()))
			{
				it = RemovePoint(it);
			}
			else
			{
				if (index > 0 && HelperButton(ImGuiDir_Up))
				{
					std::iter_swap(it, std::prev(it));
					polyline.SwapPoints(*it, *std::prev(it));
					somethingChanged = true;
				}
				if (index < points.size() - 1 && HelperButton(ImGuiDir_Down))
				{
					std::iter_swap(it, std::next(it));
					polyline.SwapPoints(*it, *std::next(it));
					somethingChanged = true;
				}
				++it;
			}

			ImGui::PopID();
			index++;
		}
		else
		{
			it = RemovePoint(it);
		}
	}

	return somethingChanged;
}

BezierCurveC2::BezierCurveC2(std::vector<std::shared_ptr<Point>> points, SelectedShapes* selectedShapes)
	:polyline({}), selectedShapes(selectedShapes), bernsteinPolyline({})
{
	observer.somethingChanged = &refreshBernstein;
	observer.moved = &movedIndex;
	renderingMode = RenderingMode::PATCHES;
	for (auto& point : points)
	{
		AddPoint(point);
	}
}

Algebra::Vector4 ScreenToNDC(float x, float y)
{
	float ndcX = (2.0f * x) / (App::windowStatic->GetWidth() - Globals::rightInterfaceWidth) - 1.0f;
	float ndcY = 1.0f - (2.0f * y) / App::windowStatic->GetHeight();

	return Algebra::Vector4(ndcX, ndcY, 0.f, 1.f);
}

void BezierCurveC2::GetClickedPoint()
{
	auto screenPos = ImGui::GetMousePos();
	auto ndcPos = ScreenToNDC(screenPos.x, screenPos.y);

	if (std::abs(ndcPos.x) > 1.f || std::abs(ndcPos.y) > 1.f)
	{
		return;
	}

	const float similarityThreshold = 0.02f;
	bool isCtrlPressed = ImGui::GetIO().KeyCtrl;

	for (const auto& shape : bezierPoints)
	{
		std::shared_ptr<RenderableOnScene> shapePtr = shape;
		if (auto point = std::dynamic_pointer_cast<Point>(shape))
		{
			Algebra::Vector4 worldPos(0.f, 0.f, 0.f, 1.f);
			Algebra::Matrix4 MVP = App::projectionMatrix * App::camera.GetViewMatrix() * point->GetModelMatrix();
			Algebra::Vector4 clipPos = MVP * worldPos;

			clipPos.z = 0.f;

			if (clipPos.w != 0.f)
			{
				clipPos = clipPos / clipPos.w;
			}

			if (std::abs(ndcPos.x - clipPos.x) < similarityThreshold &&
				std::abs(ndcPos.y - clipPos.y) < similarityThreshold)
			{
				if (isCtrlPressed)
				{
					selectedShapes->ToggleShape(shapePtr);
				}
				else
				{
					selectedShapes->Clear();
					selectedShapes->AddShape(shapePtr);
				}
			}
		}
	}
}

void BezierCurveC2::Update()
{
	if (displayBezierPoints && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		GetClickedPoint();
	}
	for (const auto& pt : bezierPoints)
	{
		pt->Update();
	}
	if (refreshBernstein)
		somethingChanged = true;
	RenderableOnScene::Update();
	polyline.Update();
	bernsteinPolyline.Update();
}

void BezierCurveC2::UpdateBasedOnDeBoor()
{
	if (points.size() < 4) return; // Need at least 4 points

	for (size_t i = 0; i + 3 < points.size(); ++i) {
		const Algebra::Vector4& P0 = points[i].lock()->GetPosition();
		const Algebra::Vector4& P1 = points[i + 1].lock()->GetPosition();
		const Algebra::Vector4& P2 = points[i + 2].lock()->GetPosition();
		const Algebra::Vector4& P3 = points[i + 3].lock()->GetPosition();

		Algebra::Vector4 B0 = (P0 + 4.0f * P1 + P2) / 6.0f;
		Algebra::Vector4 B1 = (4.0f * P1 + 2.0f * P2) / 6.0f;
		Algebra::Vector4 B2 = (2.0f * P1 + 4.0f * P2) / 6.0f;
		Algebra::Vector4 B3 = (P1 + 4.0f * P2 + P3) / 6.0f;

		bezierPoints[i * 4 + 0]->SetPosition(B0, false);
		bezierPoints[i * 4 + 1]->SetPosition(B1, false);
		bezierPoints[i * 4 + 2]->SetPosition(B2, false);
		bezierPoints[i * 4 + 3]->SetPosition(B3, false);
	}
}

void BezierCurveC2::UpdateBasedOnBernstein(int movedBezierIndex, Algebra::Vector4 deltaB)
{
	int segment = std::max((movedBezierIndex - 1), 0) / 4;

	std::shared_ptr<Point> D0 = points[segment].lock();
	std::shared_ptr<Point> D1 = points[segment + 1].lock();
	std::shared_ptr<Point> D2 = points[segment + 2].lock();
	std::shared_ptr<Point> D3 = points[segment + 3].lock();

	int modulo = movedBezierIndex % 4;
	if (modulo == 0)
	{
		D0->Move(6.f * deltaB);
	}
	else if (modulo == 1)
	{
		D1->Move(2.f * deltaB);
		D2->Move(1.f * deltaB);
	}
	else if (modulo == 2)
	{
		D1->Move(1.f * deltaB);
		D2->Move(2.f * deltaB);
	}
	else if (modulo == 3)
	{
		D3->Move(6.f * deltaB);
	}
}

