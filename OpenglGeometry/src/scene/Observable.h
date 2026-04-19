#pragma once
#include "Entity.h"
#include "Tags.h"

template <typename T>
class Observable
{
public:
	T value;
	Entity entity;

	inline T& operator=(const T& newValue)
	{
		if (value != newValue)
		{
			value = newValue;
			entity.AddTag<ObserverChangedState>();
		}
		return value;
	}

	inline operator const T& () const { return value; }
};
