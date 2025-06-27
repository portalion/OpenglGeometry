#pragma once
#include "MeshApplier.h"

template<typename T>
inline MeshApplier::MeshApplier()
	:vbo{}, ibo{} 
{
	vao = std::make_unique<VertexArray>(VertexArray::Create<T>(vbo, ibo));
}

template<typename T>
inline MeshApplier::MeshApplier(std::vector<unsigned int> indices)
	: vbo{}, ibo{ indices }
{
	vao = std::make_unique<VertexArray>(VertexArray::Create<T>(vbo, ibo));
}

template<class T>
inline MeshApplier::MeshApplier(std::vector<T> vertices)
	: vbo{ vertices }, ibo{}
{
	vao = std::make_unique<VertexArray>(VertexArray::Create<T>(vbo, ibo));
}

template<class T>
inline MeshApplier::MeshApplier(std::vector<T> vertices, std::vector<unsigned int> indices)
	: vbo{ vertices }, ibo{ indices }
{
	vao = std::make_unique<VertexArray>(VertexArray::Create<T>(vbo, ibo));
}

inline void MeshApplier::AssignIndices(std::vector<unsigned int> indices)
{
	ibo.AssignIndices(indices);
}

template<class T>
inline void MeshApplier::AssignVertices(std::vector<T> vertices)
{
	vbo.AssignVectorData(vertices);
}

inline void MeshApplier::Render(RenderingMode mode) const
{
	GLenum modeInEnum = static_cast<GLenum>(mode);
	vao->Bind();
	if (ibo.GetLength() > 0)
	{
		GLCall(glDrawElements(modeInEnum, static_cast<unsigned int>(ibo.GetLength()), GL_UNSIGNED_INT, nullptr));
	}
	else
	{
		GLCall(glDrawArrays(modeInEnum, 0, static_cast<unsigned int>(vbo.GetLength())));
	}
	vao->UnBind();
}