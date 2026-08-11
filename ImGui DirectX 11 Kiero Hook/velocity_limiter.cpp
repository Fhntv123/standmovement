#include "velocity_limiter.h"

// Forward declare Vector3 structure
struct Vector3 {
    float x, y, z;
    
    float Length2D() const {
        return sqrt(x * x + z * z);
    }
};

// Global configuration
bool velocityLimiterEnabled = false;
float velocityLimit = 500.0f;

// Apply velocity limit to horizontal speed (x, z components)
Vector3 ApplyVelocityLimit(const Vector3& velocity) {
    if (!velocityLimiterEnabled || velocityLimit <= 0.0f) {
        return velocity;
    }
    
    Vector3 result = velocity;
    float horSpeed = result.Length2D();
    
    if (horSpeed > velocityLimit) {
        float scale = velocityLimit / horSpeed;
        result.x *= scale;
        result.z *= scale;
    }
    
    return result;
}

// Clamp horizontal speed to limit
float ClampHorizontalSpeed(float currentSpeed, float limit) {
    if (!velocityLimiterEnabled || limit <= 0.0f) {
        return currentSpeed;
    }
    
    return currentSpeed > limit ? limit : currentSpeed;
}

