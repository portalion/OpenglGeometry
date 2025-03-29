#pragma once

#include "Vector4.h"

namespace Algebra
{
	class Matrix4
	{
	private:
		Vector4 rows[4];
	public:
		Matrix4();
		Matrix4(const Vector4& diagonal);
		Matrix4(const Vector4& first, const Vector4& second, const Vector4& third, const Vector4& fourth);
		Matrix4(const float& diag1, const float& diag2, const float& diag3, const float& diag4);

		float Sum() const;
		Matrix4 Transpose();
		Matrix4 Inverse();

		Vector4 Column(std::size_t index) const;

		//Operators
		Vector4& operator[](std::size_t index);
		const Vector4& operator[](std::size_t index) const;
		const Matrix4 operator+(const Matrix4& add) const;
		const Matrix4 operator-(const Matrix4& remove) const;

		const Matrix4 operator*(const Matrix4& rightMatrix) const;

		friend Matrix4 operator*(const Matrix4& matrix, const float& scale);
		friend Matrix4 operator*(const float& scale, const Matrix4& matrix);
		friend Matrix4 operator/(const Matrix4& matrix, const float& scale);

		//Static
		static Matrix4 Identity();
		static Matrix4 Translation(float x, float y, float z);
		static Matrix4 Translation(Algebra::Vector4 translation);
		static Matrix4 Rotation(float x, float y, float z);
		static Matrix4 RotationX(float angle);
		static Matrix4 RotationY(float angle);
		static Matrix4 RotationZ(float angle);
		static Matrix4 RotationDegree(float x, float y, float z);
		static Matrix4 RotationXDegree(float angle);
		static Matrix4 RotationYDegree(float angle);
		static Matrix4 RotationZDegree(float angle);
		static Matrix4 DiagonalScaling(float x, float y, float z, float w = 1);
		static Matrix4 Projection(float aspect, float lastZ, float firstZ, float fov);
	};
}