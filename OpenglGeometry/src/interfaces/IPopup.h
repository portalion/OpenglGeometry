#pragma once

class IPopup
{
public:
	virtual const char* Name() = 0;
	virtual bool ShouldOpen() = 0;
	virtual void Display() = 0;
};