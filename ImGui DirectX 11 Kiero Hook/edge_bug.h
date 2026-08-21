#pragma once

// Frame-rate independent downward acceleration applied to the displacement passed to
// CharacterController::Move. force is user-facing units/second, deltaTime is seconds.
namespace EdgeBug {
    static inline Vector3 ApplyDownwardPull(Vector3 motion, bool enabled,
        float force, float deltaTime) {
        if (!enabled || force <= 0.0f || !isfinite(deltaTime) || deltaTime <= 0.0f)
            return motion;

        motion.y -= force * deltaTime;
        return motion;
    }
}
