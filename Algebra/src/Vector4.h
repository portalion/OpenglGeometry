#pragma once
#include <cstddef>
#include <vector>
#include <ostream>

namespace Algebra
{
	class Matrix4;

	class Vector4
	{
	public:
		float x;
		float y;
		float z;
		float w;

		Vector4();
		Vector4(float x, float y, float z, float w = 0.f);

		float Length() const;
		float Sum() const;
		const Vector4 Scale(const Vector4& scalingVector) const;
		Vector4 Normalize() const;
		Vector4 Cross(Vector4 anotherVector);
		std::vector<float> ToVector() const;

		float& operator[](std::size_t index);
		const float& operator[](std::size_t index) const;
		const Vector4 operator+(const Vector4& add) const;
		const Vector4 operator-() const;
		const Vector4 operator-(const Vector4& remove) const;

		const float operator*(const Vector4& rightVector) const;
		const Vector4 operator*(const Matrix4& matrix) const;
		const bool operator==(const Vector4& v2) const;

		Vector4& operator+=(const Vector4& add);

		friend Vector4 operator*(const Vector4& vector, const float& scale);
		friend Vector4 operator*(const float& scale, const Vector4& vector);
		friend Vector4 operator/(const Vector4& vector, const float& scale);

		friend std::ostream& operator<<(std::ostream& os, const Vector4& vector);
	};
}


