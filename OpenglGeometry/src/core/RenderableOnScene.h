#pragma once
#include <string>
#include <engine/Renderer.h>

struct RenderableOnSceneMesh
{
	std::vector<PositionVertexData> vertices;
	std::vector<unsigned int> indices;
};

//TODO: ADD ID MANAGER
class RenderableOnScene
{
private:
	Renderer<PositionVertexData> renderer;
	bool somethingChanged = false;

	void SaveMesh();
protected:
	std::string name;
	RenderingMode renderingMode;

	virtual std::string GetTypeName() const = 0;
	virtual RenderableOnSceneMesh GenerateMesh() = 0;
	virtual bool DisplayParameters() = 0;
public:
	RenderableOnScene();
	virtual ~RenderableOnScene() = default;
	
	void InitName();
	void Update();
	void DisplayMenu();
	void Render() const;

	inline std::string GetName() const { return name; }
};

