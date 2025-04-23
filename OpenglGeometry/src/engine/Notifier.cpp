#include "engine/Notifier.h"

void Notifier::AddObserver(IObserver* observer)
{
	observers.push_back(observer);
}
void Notifier::RemoveObserver(IObserver* observer)
{
	observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}
void Notifier::Notify() const
{
	for (IObserver* observer : observers)
	{
		observer->OnNotify();
	}
}
