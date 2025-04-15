#pragma once
#include "core/RenderableOnScene.h"

class IObserver {
public:
	virtual ~IObserver() {};
	virtual void Update(const std::string& message_from_subject) = 0;
};

class Point : public RenderableOnScene
{
	const static RenderableMesh<PositionVertexData> mesh;

	inline std::string GetTypeName() const override { return "Point"; }
	RenderableMesh<PositionVertexData> GenerateMesh() override;
	inline bool DisplayParameters() override { return false; }
	std::vector<IObserver*> observers;
	Algebra::Matrix4 lastModel = Algebra::Matrix4::Identity();
public:
	inline Point() { renderingMode = RenderingMode::TRIANGLES; };

	void Attach(IObserver* observer) {
		observers.push_back(observer);
	}
	void Detach(IObserver* observer) {
		auto toRemove = std::find(observers.begin(), observers.end(), observer);
		if (toRemove != observers.end())
			observers.erase(toRemove);
	}
	void Notify(std::string message) 
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
	void Update() override
	{
		RenderableOnScene::Update();
		bool isEqual = true;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (std::abs(GetModelMatrix()[i][j] - lastModel[i][j]) > 0.0001f)
					isEqual = false;
			}
		}
		if (isEqual)
			return;
		lastModel = GetModelMatrix();
		Notify("Update");
	}
};

