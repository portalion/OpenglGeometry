#include "Vector4.h"
#include <stdexcept>
#include "Matrix4.h"

using namespace Algebra;

Vector4::Vector4()
	:x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 }
{
}

Vector4::Vector4(float x, float y, float z, float w)
	:x{ x }, y{ y }, z{ z }, w{ w }
{
}

float Algebra::Vector4::Length() const
{
	return sqrtf(x * x + y * y + z * z + w * w);
}

float Algebra::Vector4::Sum() const
{
	return x + y + z + w;
}

const Vector4 Algebra::Vector4::Scale(const Vector4& scalingVector) const
{
	return Vector4(this->x * scalingVector.x, this->y * scalingVector.y,
		this->z * scalingVector.z, this->w * scalingVector.w);
}

Vector4 Algebra::Vector4::Normalize() const
{
	if (Length() == 0)
	{
		return *this;
	}
	return *this / Length();
}

Vector4 Algebra::Vector4::Cross(Vector4 b)
{
	return Vector4(y * b.z - z * b.y, x * b.z - z * b.x, x * b.y - y * b.x, 0.f);
}

std::vector<float> Algebra::Vector4::ToVector() const
{
	return std::vector<float>{x, y, z, w};
}

float& Vector4::operator[](std::size_t index)
{
	switch (index) 
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2: 
		return z;
	case 3:
		return w;
	default:
		throw std::runtime_error("invalid vector4 index");
	}
}

const float& Algebra::Vector4::operator[](std::size_t index) const
{
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		throw std::runtime_error("invalid vector4 index");
	}
}

const Vector4 Algebra::Vector4::operator+(const Vector4& add) const
{
	return Vector4(this->x + add.x, this->y + add.y,
		this->z + add.z, this->w + add.w);
}

const Vector4 Algebra::Vector4::operator-() const
{
	return -1*(*this);
}

const Vector4 Algebra::Vector4::operator-(const Vector4& remove) const
{
	return Vector4(this->x - remove.x, this->y - remove.y,
		this->z - remove.z, this->w - remove.w);
}

const float Algebra::Vector4::operator*(const Vector4& rightVector) const
{
	return Scale(rightVector).Sum();
}

const bool Algebra::Vector4::operator==(const Vector4& v2) const
{
	return x == v2.x && y == v2.y && z == v2.z && w == v2.w;
}

Vector4& Algebra::Vector4::operator+=(const Vector4& add)
{
	x += add.x;
	y += add.y;
	z += add.z;
	w += add.w;

	return *this;
}

Vector4 Algebra::operator*(const Vector4& vector, const Matrix4& matrix)
{
	return Vector4(vector * matrix.Column(0), vector * matrix.Column(1),
		vector * matrix.Column(2), vector * matrix.Column(3));
}

Vector4 Algebra::operator*(const Matrix4& matrix, const Vector4& vector)
{
	return Vector4(matrix[0] * vector, matrix[1] * vector,
		matrix[2] * vector, matrix[3] * vector);
}

Vector4 Algebra::operator*(const Vector4& vector, const float& scale)
{
	return Vector4(vector.x * scale, vector.y * scale,
		vector.z * scale, vector.w * scale);
}

Vector4 Algebra::operator/(const Vector4& vector, const float& scale)
{
	if (scale == 0)
	{
		throw std::runtime_error("length of vector is 0");
	}
	return 1.f / scale * vector;
}

std::ostream& Algebra::operator<<(std::ostream& os, const Vector4& vector)
{
	os << '[' << vector.x << ' ' << vector.y << ' ' << vector.z << ' ' << vector.w << ']';
	return os;
}

Vector4 Algebra::operator*(const float& scale, const Vector4& vector)
{
	return vector * scale;
}
