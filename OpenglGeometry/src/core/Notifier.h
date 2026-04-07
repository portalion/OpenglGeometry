#pragma once
#include <vector>
#include <IObserver.h>

class Notifier
{
private:
	std::vector<IObserver*> observers;
public:
	void AddObserver(IObserver* observer);
	void RemoveObserver(IObserver* observer);
	void Notify() const;
};
