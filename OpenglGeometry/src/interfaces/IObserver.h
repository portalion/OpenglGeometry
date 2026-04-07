#pragma once

class IObserver
{
public:
	virtual ~IObserver() = default;
	virtual void OnNotify() = 0;
};