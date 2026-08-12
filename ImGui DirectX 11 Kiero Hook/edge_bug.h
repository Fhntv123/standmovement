#pragma once

// Simple downward pull applied to CharacterController::Move while Edge Bug is active.
// The 0.02 factor converts the user-facing force to an approximate per-frame motion.
namespace EdgeBug {
    static inline Vector3 ApplyDownwardPull(Vector3 motion, bool enabled, float force) {
        if (!enabled || force <= 0.0f) return motion;

        motion.y -= force * 0.02f;
        return motion;
    }
}
