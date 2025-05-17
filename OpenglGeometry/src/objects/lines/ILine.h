#pragma once
#include <memory>
#include <objects/Point.h>

class ILine
{
public:
	virtual void AddPoint(std::shared_ptr<Point> point) = 0;
};
