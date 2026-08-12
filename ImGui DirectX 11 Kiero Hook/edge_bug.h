#pragma once

// Edge bug assistance for Unity CharacterController movement.
// When a falling controller briefly reports grounded on a ledge, suppress that
// grounded frame and remove only the downward component of the next Move call.
// Horizontal movement is deliberately left untouched.
namespace EdgeBug {
    struct State {
        float verticalVelocity;
        bool falling;
        bool pendingLandingClamp;

        State() : verticalVelocity(0.0f), falling(false), pendingLandingClamp(false) {}
    };

    static inline State& GetState() {
        static State state;
        return state;
    }

    static inline void Reset() {
        GetState() = State();
    }

    static inline void ObserveVelocity(const Vector3& velocity, bool enabled, float minFallSpeed) {
        State& state = GetState();
        if (!enabled) {
            Reset();
            return;
        }

        state.verticalVelocity = velocity.y;
        state.falling = velocity.y <= -minFallSpeed;
    }

    static inline bool FilterGrounded(bool grounded, bool enabled) {
        State& state = GetState();
        if (!enabled) return grounded;

        if (grounded && state.falling) {
            state.pendingLandingClamp = true;
            return false;
        }

        return grounded;
    }

    static inline Vector3 ClampLandingMotion(Vector3 motion, bool enabled) {
        State& state = GetState();
        if (!enabled || !state.pendingLandingClamp) return motion;

        if (motion.y < 0.0f) motion.y = 0.0f;
        state.pendingLandingClamp = false;
        state.falling = false;
        return motion;
    }
}
