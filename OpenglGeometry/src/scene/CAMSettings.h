#pragma once
#include <array>

struct MaterialSettings
{
	std::array<float, 3> m_Size{ 150.f, 150.f, 50.f };
	std::array<int, 2> m_Resolution{ 1000, 1000 };
	float m_MaxDepth = 20.f;
};

enum class SimulationState
{
	Stopped,
	Running,
	Paused,
	Finished
};

struct SimulationSettings
{
	float m_Speed = 50.f;
	SimulationState m_State = SimulationState::Stopped;
	float m_Progress = 0.f;
	bool m_FinishRequested = false;
	bool m_ResetRequested = false;
};

struct CAMSettings
{
	MaterialSettings m_Material;
	SimulationSettings m_Simulation;
};
