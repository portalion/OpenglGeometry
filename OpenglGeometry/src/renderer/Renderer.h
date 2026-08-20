#pragma once
#include "core/Base.h"
#include <managers/ShaderManager.h>
#include "VertexArray.h"
#include <GL/glew.h>
#include "RendererContext.h"

enum class RenderingMode
{
	Triangles = GL_TRIANGLES,
	Lines = GL_LINES,
	Patches = GL_PATCHES
};

enum class DepthMode
{
	NoDepth = GL_FALSE,
	Depth = GL_TRUE
};

class Renderer
{
private:
	Ref<Shader> m_ActualShader;
	Algebra::Matrix4 m_ActualProjectionMatrix;
	Algebra::Matrix4 m_ActualViewMatrix;

	Ref<VertexArray> m_ActualMesh;

	UniformContext sceneContext;
public:
	void SetShader(AvailableShaders shaderType);
	void SetSceneContext(SceneContext context);
	void SetMesh(Ref<VertexArray> mesh);
	void SetDepthMode(DepthMode depthMode);
	void Render(RenderingMode mode, const EntityContext& context);
};

