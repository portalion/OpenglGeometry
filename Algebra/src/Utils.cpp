#include "Utils.h"
#define _USE_MATH_DEFINES
#include <cmath>

float Algebra::DegreeToRadians(float degree)
{
    return M_PI * degree / 180.f;
}

float Algebra::RadiansToDegree(float radians)
{
    return 180.f * radians / M_PI;
}
