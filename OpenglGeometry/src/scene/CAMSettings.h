#pragma once
#include <array>
#include <optional>
#include "managers/IdManager.h"

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
	std::optional<ID> m_ActivePath; // path being milled, chosen with its Start button
	float m_Progress = 0.f;
	bool m_FinishRequested = false;
	bool m_ResetRequested = false;
	bool m_ShowTool = true;
};

struct CAMSettings
{
	MaterialSettings m_Material;
	SimulationSettings m_Simulation;
};
