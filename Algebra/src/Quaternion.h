#pragma once
#include <cmath>
#include <stdexcept>
#include "Matrix4.h"
#include "Vector4.h"
#include <numbers>
#include "Utils.h"
namespace Algebra
{
    struct EulerAngles
    {
        float pitch;
        float yaw;
        float roll;
    };
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
        EulerAngles ToEulerAngles(Quaternion q) {
            EulerAngles angles;

            // roll (x-axis rotation)
            float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
            float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
            angles.roll = RadiansToDegree(std::atan2(sinr_cosp, cosr_cosp));

            // pitch (y-axis rotation)
            float sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
            float cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
            angles.pitch = RadiansToDegree(2 * std::atan2(sinp, cosp) - std::numbers::pi_v<float> / 2);

            // yaw (z-axis rotation)
            float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
            float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
            angles.yaw = RadiansToDegree(std::atan2(siny_cosp, cosy_cosp));

            return angles;
        }
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

