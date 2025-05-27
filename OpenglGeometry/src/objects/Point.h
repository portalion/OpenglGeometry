#pragma once
#include "core/RenderableOnScene.h"

class IObserver {
public:
	virtual ~IObserver() {};
	virtual void Update(const std::string& message_from_subject) = 0;
};

class Point : public RenderableOnScene
{
protected:
	const static RenderableMesh<PositionVertexData> mesh;

	inline std::string GetTypeName() const override { return "Point"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	inline bool DisplayParameters() override { return false; }
	std::vector<IObserver*> observers;
	Algebra::Vector4 lastPos;
	Algebra::Vector4 movedPos;
public:
	bool removable = true;
	inline Point() { renderingMode = RenderingMode::TRIANGLES; };
	virtual ~Point() { Notify("Destructor"); }
	void Attach(IObserver* observer) {
		observers.push_back(observer);
	}
	void Detach(IObserver* observer) {
		auto toRemove = std::find(observers.begin(), observers.end(), observer);
		if (toRemove != observers.end())
			observers.erase(toRemove);
	}
	virtual void Notify(std::string message)
	{
		auto it = observers.begin();

		while (it != observers.end()) {
			if (*it == nullptr)
			{
				it = observers.erase(it);
				continue;
			}
			(*it)->Update(message);
			++it;
		}
	}
	inline void Update() override
	{
		RenderableOnScene::Update();
		bool isEqual = true;
		for (int i = 0; i < 4; i++)
		{
			if (std::abs(GetPosition()[i] - lastPos[i]) > 0.0001f)
			{
				isEqual = false;
				break;
			}
		}
		if (isEqual)
			return;
		Notify("Update");
		movedPos = GetPosition() - lastPos;
		lastPos = GetPosition();
	}

	json Serialize() const override;
};


class BernPoint : public Point
{
	int idx;
public:
	BernPoint(int i) : idx(i) {};
	virtual ~BernPoint() override { Notify(std::to_string(-1)); }
	Algebra::Vector4 GetMoved() {
		return movedPos;
	}
	void Notify(std::string message) override
	{
		Point::Notify(std::to_string(idx));
	}
	void SetPosition(Algebra::Vector4 pos, bool v)
	{
		Point::SetPosition(pos);
		lastPos = Point::GetPosition();
		Point::Notify(std::to_string(-1));
	}

	void Move(Algebra::Vector4 delta)
	{
		Point::Move(delta);
		lastPos = Point::GetPosition();
		Point::Notify(std::to_string(-1));
	}
};
