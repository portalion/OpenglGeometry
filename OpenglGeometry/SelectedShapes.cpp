#include "SelectedShapes.h"
#include "core/Transformable.h"

void SelectedShapes::Clear()
{
	selectedShapes.clear();
}

void SelectedShapes::AddShape(std::shared_ptr<RenderableOnScene> shapePtr)
{
	if (IsSelected(shapePtr))
	{
		return;
	}
	selectedShapes.push_back(shapePtr);
}

void SelectedShapes::RemoveShape(std::shared_ptr<RenderableOnScene> shapePtr)
{
	selectedShapes.erase(std::find(selectedShapes.begin(), selectedShapes.end(), shapePtr));
}

void SelectedShapes::ToggleShape(std::shared_ptr<RenderableOnScene> shapePtr)
{
	if (IsSelected(shapePtr))
	{
		RemoveShape(shapePtr);
	}
	else
	{
		AddShape(shapePtr);
	}
}

bool SelectedShapes::IsSelected(std::shared_ptr<RenderableOnScene> shapePtr) const
{
	return std::find(selectedShapes.begin(), selectedShapes.end(), shapePtr) != selectedShapes.end();
}

std::optional<Algebra::Vector4> SelectedShapes::GetAveragePosition() const
{
	if (IsEmpty())
	{
		return std::nullopt;
	}

	Algebra::Vector4 result;
	auto selectedWithPosition = GetSelectedWithType<PositionComponent>();
	for (auto& selected : selectedWithPosition)
	{
		result += selected->GetPosition();
	}
	result = result / selectedWithPosition.size();
	return result;
}
