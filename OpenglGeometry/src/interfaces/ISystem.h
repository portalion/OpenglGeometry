#pragma once

class ISystem
{
public:
	virtual ~ISystem() = default;
	virtual void Process() = 0;
};
