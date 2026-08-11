#pragma once

#include <cmath>

struct Vector3;

// Velocity limiter configuration
extern bool velocityLimiterEnabled;
extern float velocityLimit;

// Apply velocity limit to horizontal speed (x, z components)
// Returns the clamped velocity
Vector3 ApplyVelocityLimit(const Vector3& velocity);

// Clamp horizontal speed to limit
float ClampHorizontalSpeed(float currentSpeed, float limit);

