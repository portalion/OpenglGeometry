#include "InterpolatedBezierCurve.h"
#include <algorithm>
#include "App.h"
#include "core/Globals.h"

std::vector<Algebra::Vector4> SolveTrilinearMatrix(std::vector<float> alpha, std::vector<float> beta, std::vector <Algebra::Vector4> r)
{
	const std::size_t m = r.size();
	std::vector<float> gamma(m);
	std::vector<Algebra::Vector4> delta(m), c(m);

	if (r.size() == 1)
	{
		c[0] = r[0] / 2.f;
		c[0].w = 0.0f;
		return c;
	}

	float denom = 2.0f;
	gamma[0] = beta[0] / denom;
	delta[0] = r[0] / denom;
	delta[0].w = 0.0f;

	for (std::size_t i = 1; i < m; ++i) {
		denom = 2.0f - alpha[i - 1] * gamma[i - 1];
		if (i < m - 1) {
			gamma[i] = beta[i] / denom;
		}
		Algebra::Vector4 tmp = r[i] - alpha[i - 1] * delta[i - 1];
		delta[i] = tmp / denom;
		delta[i].w = 0.0f;
	}

	c[m - 1] = delta[m - 1];
	c[m - 1].w = 0.0f;
	for (int i = static_cast<int>(m) - 2; i >= 0; --i) {
		Algebra::Vector4 tmp = delta[i] - gamma[i] * c[i + 1];
		c[i] = tmp;
		c[i].w = 0.0f;
	}

	return c;
}

RenderableMesh<PositionVertexData> InterpolatedBezierCurve::GenerateMesh()
{
	RenderableMesh<PositionVertexData> mesh;

	std::vector<Algebra::Vector4> positions;
	std::vector<float> d;
	std::vector<float> alpha;
	std::vector<float> beta;
	std::vector<Algebra::Vector4> r;

	for (const auto& p : points)
	{
		if (auto point = p.lock())
		{
			positions.push_back(point->GetPosition());
		}
	}

	if (positions.size() == 2)
	{
		mesh.vertices.push_back(PositionVertexData{ .Position = positions[0] });
		mesh.vertices.push_back(PositionVertexData{ .Position = positions[0] });
		mesh.vertices.push_back(PositionVertexData{ .Position = positions[1] });
		mesh.vertices.push_back(PositionVertexData{ .Position = positions[1] });
		return mesh;
	}
	else if (positions.size() < 2) return {};

	for (int i = 0; i < positions.size() - 1; i++)
	{
		d.push_back((positions[i + 1] - positions[i]).Length());
	}

	for (int i = 1; i < d.size(); i++)
	{
		float d0 = d[i - 1];
		float d1 = d[i];
		if(i != 1)
			alpha.push_back(d0 / (d0 + d1));
		if (i != d.size() - 1)
			beta.push_back(d1 / (d0 + d1));

		Algebra::Vector4 P0 = (positions[i] - positions[i - 1]) / d0;
		Algebra::Vector4 P1 = (positions[i + 1] - positions[i]) / d1;

		r.push_back(3.f * (P1 - P0) / (d0 + d1));
	}

	for (int i = 0; i < alpha.size(); i++)
	{
		if (std::isnan(alpha[i])) alpha[i] = 0.f;
		if (std::isnan(beta[i])) beta[i] = 0.f;
	}

	auto c = SolveTrilinearMatrix(alpha, beta, r);
	c.insert(c.begin(), Algebra::Vector4());
	c.push_back(Algebra::Vector4());
	std::vector<Algebra::Vector4> a(c.size());
	std::vector<Algebra::Vector4> b(c.size());
	std::vector<Algebra::Vector4> D(c.size());

	for (int i = 0; i < d.size(); i++)
		D[i] = (c[i + 1] - c[i]) / d[i] / 3.f;

	for (int i = 0; i < a.size(); i++)
		a[i] = positions[i];

	for (int i = 0; i < b.size() - 1; i++)
		b[i] = (a[i + 1] - a[i]) / d[i] - c[i] * d[i] - D[i] * d[i] * d[i];

	for (size_t i = 0; i < d.size(); ++i) {
		Algebra::Vector4 ai = a[i];
		Algebra::Vector4 bi = b[i] * d[i];
		Algebra::Vector4 ci = c[i] * d[i] * d[i];
		Algebra::Vector4 di = D[i] * d[i] * d[i] * d[i];

		Algebra::Vector4 P0 = ai;
		Algebra::Vector4 P1 = ai + bi / 3.f;
		Algebra::Vector4 P2 = ai + bi * 2.f / 3.f + ci / 3.f;
		Algebra::Vector4 P3 = ai + bi + ci + di;
		P0.w = P1.w = P2.w = P3.w = 1.0f;

		mesh.vertices.push_back(PositionVertexData{ .Position = P0 });
		mesh.vertices.push_back(PositionVertexData{ .Position = P1 });
		mesh.vertices.push_back(PositionVertexData{ .Position = P2 });
		mesh.vertices.push_back(PositionVertexData{ .Position = P3 });
	}
	
	return mesh;
}

bool InterpolatedBezierCurve::HelperButton(ImGuiDir direction)
{
	ImGui::SameLine();
	return ImGui::ArrowButton(GenerateLabelWithId(std::to_string(static_cast<int>(direction))).c_str(), direction);
}

bool InterpolatedBezierCurve::DisplayParameters()
{
	SetPosition(Algebra::Vector4());
	SetRotation(Algebra::Quaternion());
	SetScale(1.f);

	ImGui::Checkbox(this->GenerateLabelWithId("Draw Polyline").c_str(), &displayPolyline);
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
	ImGui::Text("Bezier Control Points (%zu)", points.size());

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
				polyline.RemovePoint(*it);
				it->lock()->Detach(this);
				it = points.erase(it);
				somethingChanged = true;
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

			polyline.RemovePoint(*it);
			it = points.erase(it);
			somethingChanged = true;
		}
	}

	return somethingChanged;
}

InterpolatedBezierCurve::InterpolatedBezierCurve(std::vector<std::shared_ptr<Point>> points, SelectedShapes* selectedShapes)
	:polyline({}), selectedShapes(selectedShapes)
{
	renderingMode = RenderingMode::PATCHES;
	for (auto& point : points)
	{
		AddPoint(point);
	}
}

