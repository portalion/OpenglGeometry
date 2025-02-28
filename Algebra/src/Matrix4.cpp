#include "Matrix4.h"

#include <stdexcept>
#include "Utils.h"

using namespace Algebra;

Matrix4::Matrix4()
{
	rows[0] = Vector4();
	rows[1] = Vector4();
	rows[2] = Vector4();
	rows[3] = Vector4();
}

Matrix4::Matrix4(const Vector4& diagonal)
	:Matrix4{}
{
	rows[0][0] = diagonal[0];
	rows[1][1] = diagonal[1];
	rows[2][2] = diagonal[2];
	rows[3][3] = diagonal[3];
}

Algebra::Matrix4::Matrix4(const float& diag1, const float& diag2, const float& diag3, const float& diag4)
	:Matrix4(Vector4(diag1, diag2, diag3, diag4))
{
}

Matrix4::Matrix4(const Vector4& first, const Vector4& second, 
	const Vector4& third, const Vector4& fourth)
{
	rows[0] = first;
	rows[1] = second;
	rows[2] = third;
	rows[3] = fourth;
}

float Matrix4::Sum() const
{
	float result = 0.f;
	for (int i = 0; i < 4; i++)
		result += rows[i].Sum();
	return result;
}

Matrix4 Algebra::Matrix4::Transpose()
{
	return Matrix4(this->Column(0), this->Column(1), this->Column(2), this->Column(3));
}

Vector4 Matrix4::Column(std::size_t index) const
{
	return Vector4(rows[0][index], rows[1][index], rows[2][index], rows[3][index]);
}

Vector4& Matrix4::operator[](std::size_t index)
{
	return rows[index];
}

const Vector4& Matrix4::operator[](std::size_t index) const
{
	return rows[index];
}

const Matrix4 Matrix4::operator+(const Matrix4& add) const
{
	return Matrix4(this->rows[0] + add[0], this->rows[1] + add[1],
		this->rows[2] + add[2], this->rows[3] + add[3]);
}

const Matrix4 Matrix4::operator-(const Matrix4& remove) const
{
	return Matrix4(this->rows[0] - remove[0], this->rows[1] - remove[1],
		this->rows[2] - remove[2], this->rows[3] - remove[3]);
}

const Matrix4 Algebra::Matrix4::operator*(const Matrix4& rightMatrix) const
{
	Matrix4 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i][j] = (*this)[i] * rightMatrix.Column(j);
		}
	}
	return result;
}

Matrix4 Algebra::operator*(const Matrix4& matrix, const float& scale)
{
	return Matrix4(matrix[0] * scale, matrix[1] * scale, matrix[2] * scale, matrix[3] * scale);
}

Matrix4 Algebra::operator*(const float& scale, const Matrix4& matrix)
{
	return matrix * scale;
}

Matrix4 Algebra::operator/(const Matrix4& matrix, const float& scale)
{
	if (scale == 0)
	{
		throw std::runtime_error("matrix cannot divide by scalar 0");
	}
	return matrix * (1.f / scale);
}

Matrix4 Algebra::Matrix4::Identity()
{
	return Matrix4(1, 1, 1, 1);
}

Matrix4 Algebra::Matrix4::Translation(float x, float y, float z)
{
	Matrix4 result = Matrix4::Identity();
	result[0][3] = x;
	result[1][3] = y;
	result[2][3] = z;
	return result;
}

Matrix4 Algebra::Matrix4::Rotation(float x, float y, float z)
{
	return RotationX(x) * RotationY(y) * RotationZ(z);
}

Matrix4 Algebra::Matrix4::RotationX(float angle)
{
	Matrix4 result = Matrix4::Identity();
	float cosOfAngle = cos(angle);
	float sinOfAngle = sin(angle);

	result[1][1] = cosOfAngle;
	result[1][2] = -sinOfAngle;
	result[2][1] = sinOfAngle;
	result[2][2] = cosOfAngle;

	return result;
}

Matrix4 Algebra::Matrix4::RotationY(float angle)
{
	Matrix4 result = Matrix4::Identity();
	float cosOfAngle = cos(angle);
	float sinOfAngle = sin(angle);

	result[0][0] = cosOfAngle;
	result[0][2] = sinOfAngle;
	result[2][0] = -sinOfAngle;
	result[2][2] = cosOfAngle;

	return result;
}

Matrix4 Algebra::Matrix4::RotationZ(float angle)
{
	Matrix4 result = Matrix4::Identity();
	float cosOfAngle = cos(angle);
	float sinOfAngle = sin(angle);

	result[0][0] = cosOfAngle;
	result[0][1] = -sinOfAngle;
	result[1][0] = sinOfAngle;
	result[1][1] = cosOfAngle;

	return result;
}

Matrix4 Algebra::Matrix4::RotationDegree(float x, float y, float z)
{
	return RotationXDegree(x) * RotationYDegree(y) * RotationZDegree(z);
}

Matrix4 Algebra::Matrix4::RotationXDegree(float angle)
{
	return RotationX(DegreeToRadians(angle));
}

Matrix4 Algebra::Matrix4::RotationYDegree(float angle)
{
	return RotationY(DegreeToRadians(angle));
}

Matrix4 Algebra::Matrix4::RotationZDegree(float angle)
{
	return RotationZ(DegreeToRadians(angle));
}

Matrix4 Algebra::Matrix4::DiagonalScaling(float x, float y, float z, float w)
{
	return Matrix4(x, y, z, w);
}
