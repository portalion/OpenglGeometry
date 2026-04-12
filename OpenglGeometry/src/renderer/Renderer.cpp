#include "Renderer.h"
#include <GL/glew.h>

void Renderer::SetShader(AvailableShaders shaderType)
{
	m_ActualShader = ShaderManager::GetInstance().GetShader(shaderType);
	m_ActualShader->Bind();
}

void Renderer::SetSceneContext(SceneContext context)
{
	UniformContext sceneContextTemp;
	sceneContextTemp.Matrix4Uniforms["g_projectionMatrix"] = context.ProjectionMatrix;
	sceneContextTemp.Matrix4Uniforms["g_viewMatrix"] = context.ViewMatrix;
	sceneContextTemp.Vector4Uniforms["g_cameraPosition"] = context.CameraPosition;

	sceneContext = sceneContextTemp;
}

void Renderer::SetMesh(Ref<VertexArray> mesh)
{
	m_ActualMesh = mesh;
}

void Renderer::Render(RenderingMode mode, const EntityContext& context)
{
	if (!m_ActualShader) return;

	UniformContext uniformContext = sceneContext;
	uniformContext.Matrix4Uniforms["u_modelMatrix"] = context.Position * context.Rotation * context.Scale;
	uniformContext.Vector4Uniforms["u_color"] = Algebra::Vector4(1.f, 0.2f, 0.f, 1.f);

	m_ActualShader->ApplyContext(uniformContext);

	m_ActualMesh->Bind();

	glDrawElements(static_cast<GLenum>(mode), m_ActualMesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}
