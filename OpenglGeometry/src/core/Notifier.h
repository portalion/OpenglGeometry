#pragma once
#include <vector>
#include <interfaces/IObserver.h>

class Notifier
{
private:
	std::vector<IObserver*> observers;
public:
	void AddObserver(IObserver* observer);
	void RemoveObserver(IObserver* observer);
	void Notify() const;
};
