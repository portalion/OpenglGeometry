#pragma once
#include <cmath>
#include <stdexcept>
#include "Matrix4.h"
#include "Vector4.h"

namespace Algebra
{
	class Quaternion
	{
	public:
        float w, x, y, z;

        Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
        Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

        friend std::ostream& operator<<(std::ostream& os, const Quaternion& quaternion);

        static Quaternion Identity() { return Quaternion(1.0f, 0.0f, 0.0f, 0.0f); }

        static Quaternion CreateFromAxisAngle(const Vector4& axis, float angle);
        static Quaternion CreateFromEulerAngles(float yaw, float pitch, float roll);

        float Length() const;
        Quaternion Normalize() const;
        Quaternion Conjugate() const;
        Quaternion Inverse() const;
        float Dot(const Quaternion& other) const;
        Matrix4 ToMatrix() const;
        Vector4 Rotate(const Vector4& vector) const;

        Quaternion operator*(const Quaternion& other) const;
        Quaternion operator+(const Quaternion& other) const;
        Quaternion operator*(float scalar) const;
        Quaternion& operator*=(float scalar);
        bool operator==(const Quaternion& other) const;
	};
}

