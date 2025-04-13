#pragma once
#include <vector>
#include <memory>
#include <core/RenderableOnScene.h>
#include <optional>

class SelectedShapes
{
private:
	std::vector<std::shared_ptr<RenderableOnScene>> selectedShapes;
public:
	void Clear();
	void AddShape(std::shared_ptr<RenderableOnScene> shapePtr);
	void RemoveShape(std::shared_ptr<RenderableOnScene> shapePtr);
	void ToggleShape(std::shared_ptr<RenderableOnScene> shapePtr);
	bool IsSelected(std::shared_ptr<RenderableOnScene> shapePtr) const;
	inline bool IsEmpty() const { return selectedShapes.empty(); }
	std::optional<Algebra::Vector4> GetAveragePosition() const;

	template<typename T>
	inline std::vector<std::shared_ptr<T>> GetSelectedWithType() const;
};

template<typename T>
inline std::vector<std::shared_ptr<T>> SelectedShapes::GetSelectedWithType() const
{
	{
		std::vector<std::shared_ptr<T>> selected;
		for (const auto& shape : selectedShapes)
		{
			if (std::shared_ptr<T> castedShape = std::dynamic_pointer_cast<T>(shape))
			{
				selected.push_back(castedShape);
			}
		}
		return selected;
	}
}
