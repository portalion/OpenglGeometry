#include "Globals.h"

const int Globals::startingSceneWidth = 1280;
const int Globals::startingSceneHeight = 960;
const int Globals::rightInterfaceWidth = 400;

const float Globals::cameraNearPlane = 0.1f;
const float Globals::cameraFarPlane = 1000.f;
const float Globals::cameraFieldOfView = 3.14f / 4.f;

const Algebra::Vector4 Globals::startingCameraTarget = Algebra::Vector4(0.f, 0.f, 0.f, 1.f);
const float Globals::startingCameraDistance = 10.f;
const float Globals::startingCameraPitch = 0.3f;

const Algebra::Vector4 Globals::defaultPointsColor = Algebra::Vector4(1.f, 0.2f, 0.f, 1.f);
const Algebra::Vector4 Globals::selectionColor = Algebra::Vector4(1.f, 0.85f, 0.15f, 1.f);
const Algebra::Vector4 Globals::bernsteinPointColor = Algebra::Vector4(0.2f, 0.75f, 1.f, 1.f);