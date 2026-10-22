#pragma once
#include <vector>
#include <optional>
#include <chrono>
#include "core/Base.h"
#include "interfaces/ISystem.h"
#include "scene/CAMSettings.h"
#include "HeightMap.h"
#include "MaterialBlock.h"
#include "ToolModel.h"
#include "renderer/StorageBuffer.h"
#include "managers/IdManager.h"

class Scene;
struct PathComponent;
struct Tool;

class SimulationSystem : public ISystem
{
private:
	Ref<Scene> m_Scene;
	Ref<CAMSettings> m_Settings;

	HeightMap m_HeightMap;
	MaterialBlock m_Block;
	ToolModel m_ToolModel;

	StorageBuffer m_PathBuffer;
	std::chrono::steady_clock::time_point m_LastFrame = std::chrono::steady_clock::now();

	// Tool position along the active path
	std::optional<ID> m_ToolPathId;
	size_t m_ToolSegment = 0;			// index of the point the current segment starts at
	float m_ToolSegmentOffset = 0.f;	// distance travelled along the current segment
	float m_TravelledLength = 0.f;
	float m_PathLength = 0.f;
	Algebra::Vector4 m_ToolPosition;	// tool tip, path file coordinates
	bool m_ToolPlaced = false;

	void ResetMaterial();
	float TickFrameTime();
	void Simulate(float deltaTime);
	void UpdateToolModel();
	const PathComponent* FindActivePath() const;
	void ResetTool(const std::vector<Algebra::Vector4>& path);
	std::vector<Algebra::Vector4> AdvanceTool(const std::vector<Algebra::Vector4>& path, float distance);
	void ProcessHeightMap(const std::vector<Algebra::Vector4>& sweptPoints, const Tool& tool);
public:
	SimulationSystem(Ref<Scene> scene, Ref<CAMSettings> settings);

	void Process() override;
};
