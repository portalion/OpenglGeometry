#pragma once
#include "VertexData.h"

template<typename T>
struct VertexAttribLayout;

template<>
struct VertexAttribLayout<PositionVertexData> {
	static void Enable() {
		GLCall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PositionVertexData),
			(void*)offsetof(PositionVertexData, Position)));
		GLCall(glEnableVertexAttribArray(0));
	}
};

template<>
struct VertexAttribLayout<PositionColorVertexData> {
	static void Enable() {
		GLCall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PositionColorVertexData),
			(void*)offsetof(PositionColorVertexData, Position)));
		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(PositionColorVertexData),
			(void*)offsetof(PositionColorVertexData, Color)));
		GLCall(glEnableVertexAttribArray(1));
	}
};

template<>
struct VertexAttribLayout<PositionNormalVertexData> {
	static void Enable() {
		GLCall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PositionNormalVertexData),
			(void*)offsetof(PositionNormalVertexData, Position)));
		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(PositionNormalVertexData),
			(void*)offsetof(PositionNormalVertexData, Normal)));
		GLCall(glEnableVertexAttribArray(1));
	}
};
