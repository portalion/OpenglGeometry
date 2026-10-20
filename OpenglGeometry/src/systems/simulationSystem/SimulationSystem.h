#pragma once
#include <vector>
#include <chrono>
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include "scene/CAMSettings.h"
#include "HeightMap.h"
#include "MaterialBlock.h"

class Scene;

class SimulationSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<CAMSettings> m_Settings;

	HeightMap m_HeightMap;
	MaterialBlock m_Block;

	// Where the tool is: path, segment within it, and millimetres travelled along that segment.
	std::size_t m_PathIndex = 0;
	std::size_t m_SegmentIndex = 0;
	float m_SegmentDistance = 0.f;
	std::chrono::steady_clock::time_point m_LastStep = std::chrono::steady_clock::now();

	void ResetMaterial();
	// Moves the tool up to distance [mm] along the paths, milling as it goes; returns true once every path is done.
	bool Advance(float distance);
public:
	SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings);

	void Process() override;
};
