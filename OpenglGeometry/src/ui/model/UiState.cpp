#include "UiState.h"

#include <algorithm>

std::size_t UiState::SelectedCount() const
{
	return static_cast<std::size_t>(
		std::ranges::count_if(objects, [](const ObjectRow& row) { return row.selected; }));
}

std::vector<const ObjectRow*> UiState::Selected() const
{
	std::vector<const ObjectRow*> result;

	for (const ObjectRow& row : objects)
	{
		if (row.selected)
		{
			result.push_back(&row);
		}
	}

	return result;
}

const ObjectRow* UiState::Find(uint32_t id) const
{
	for (const ObjectRow& row : objects)
	{
		if (row.id == id)
		{
			return &row;
		}
	}

	return nullptr;
}

uint32_t UiState::AppendObject(ObjectType type, const std::string& namePrefix)
{
	uint32_t maxId = 0;

	for (const ObjectRow& row : objects)
	{
		maxId = std::max(maxId, row.id);
	}

	ObjectRow row;
	row.id = maxId + 1;
	row.name = namePrefix + " " + std::to_string(row.id);
	row.type = type;

	objects.push_back(row);
	return row.id;
}
