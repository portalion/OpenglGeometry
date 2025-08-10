#pragma once
#include "core/Base.h"
#include <managers/ShaderManager.h>
#include "VertexArray.h"
#include <GL/glew.h>

enum class RenderingMode
{
	Triangles = GL_TRIANGLES
};

class Renderer
{
private:
	Ref<Shader> m_ActualShader;
	Algebra::Matrix4 m_ActualProjectionMatrix;
	Algebra::Matrix4 m_ActualViewMatrix;
	Algebra::Matrix4 m_ActualModelMatrix;

	Ref<VertexArray> m_ActualMesh;
public:
	void SetShader(AvailableShaders shaderType);
	void SetCamera(Algebra::Matrix4 projectionMatrix, Algebra::Matrix4 viewMatrix);
	void SetMesh(Ref<VertexArray> mesh);
	void SetTransformations(Algebra::Matrix4 modelMatrix);
	void Render(RenderingMode mode);
};

