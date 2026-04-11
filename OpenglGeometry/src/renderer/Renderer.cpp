#include "Renderer.h"
#include <GL/glew.h>

void Renderer::SetShader(AvailableShaders shaderType)
{
	m_ActualShader = ShaderManager::GetInstance().GetShader(shaderType);
	m_ActualShader->Bind();
}

void Renderer::SetCamera(Algebra::Matrix4 projectionMatrix, Algebra::Matrix4 viewMatrix)
{
	m_ActualProjectionMatrix = projectionMatrix;
	m_ActualViewMatrix = viewMatrix;
}

void Renderer::SetMesh(Ref<VertexArray> mesh)
{
	m_ActualMesh = mesh;
}

void Renderer::Render(RenderingMode mode, const RendererContext& context)
{
	if (!m_ActualShader) return;

	m_ActualShader->SetUniformMat4f("u_projectionMatrix", m_ActualProjectionMatrix);
	m_ActualShader->SetUniformMat4f("u_viewMatrix", m_ActualViewMatrix);
	m_ActualShader->SetUniformMat4f("u_modelMatrix", context.Position * context.Rotation * context.Scale);
	
	m_ActualMesh->Bind();

	glDrawElements(static_cast<GLenum>(mode), m_ActualMesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}
