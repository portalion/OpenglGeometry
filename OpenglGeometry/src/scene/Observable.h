#pragma once
#include "Entity.h"
#include "Tags.h"

template <typename T>
class Observable
{
public:
	T value;
	Entity entity;

	void Set(const T& newValue)
	{
		if (value != newValue)
		{
			value = newValue;
			entity.AddTag<IsNotifiedTag>();
		}
	}
	operator const T& () const { return value; }
};
