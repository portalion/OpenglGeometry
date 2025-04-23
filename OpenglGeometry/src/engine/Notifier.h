#pragma once
#include <vector>

class IObserver
{
public:
	virtual ~IObserver() = default;
	virtual void OnNotify() = 0;
};

class Notifier
{
private:
	std::vector<IObserver*> observers;
public:
	void AddObserver(IObserver* observer);
	void RemoveObserver(IObserver* observer);
	void Notify();
}
