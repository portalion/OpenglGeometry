#include "Utils.h"
#define _USE_MATH_DEFINES
#include <cmath>

float Algebra::DegreeToRadians(float degree)
{
    return static_cast<float>(M_PI * degree / 180.f);
}

float Algebra::RadiansToDegree(float radians)
{
    return static_cast<float>(180.f * radians / M_PI);
}
