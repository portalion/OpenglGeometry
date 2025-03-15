#pragma once
#include "Renderer.h"

template<class T>
inline Renderer<T>::Renderer(VertexDataType dataType)
	:vbo{}, ibo{}, vao{ vbo, dataType, ibo }
{
}

template<class T>
inline Renderer<T>::Renderer(VertexDataType dataType, std::vector<unsigned int> indices)
	: vbo{}, ibo{ indices }, vao{ vbo, dataType, ibo }
{
}

template<class T>
inline Renderer<T>::Renderer(VertexDataType dataType, std::vector<T> vertices)
	:vbo{ vertices }, ibo{}, vao{ vbo, dataType, ibo }
{
}

template<class T>
inline Renderer<T>::Renderer(VertexDataType dataType, std::vector<T> vertices, std::vector<unsigned int> indices)
	:vbo{ vertices }, ibo{ indices }, vao{ vbo, dataType, ibo }
{
}

template<class T>
inline void Renderer<T>::AssignIndices(std::vector<unsigned int> indices)
{
	ibo.AssignIndices(indices);
}

template<class T>
inline void Renderer<T>::AssignVertices(std::vector<T> vertices)
{
	vbo.AssignVectorData(vertices);
}

template<class T>
inline void Renderer<T>::Render(GLenum mode)
{
	vao.Bind();
	if (ibo.GetLength() > 0)
	{
		GLCall(glDrawElements(mode, static_cast<unsigned int>(ibo.GetLength()), GL_UNSIGNED_INT, nullptr));
	}
	else
	{
		GLCall(glDrawArrays(mode, 0, static_cast<unsigned int>(vbo.GetLength())));
	}
	vao.UnBind();
}
