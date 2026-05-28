#include "Camera.h"
#include "Renderer.h"
#include <cmath>

void updateCamera(Camera3D& camera, const std::vector<Body>& bodies, int& followIndex) {
    static float camYaw      = 0.0f;
    static float camPitch    = 0.7f;
    static float camDistance = 85.0f;

    const float rotateSpeed = 1.5f;
    const float zoomSpeed   = 1.1f;
    float frameTime = GetFrameTime();

    // --- Follow target selection (0 = Sun, 1-8 = planets) ---
    if (IsKeyPressed(KEY_ZERO))  followIndex = 0;
    if (IsKeyPressed(KEY_ONE))   followIndex = 1;
    if (IsKeyPressed(KEY_TWO))   followIndex = 2;
    if (IsKeyPressed(KEY_THREE)) followIndex = 3;
    if (IsKeyPressed(KEY_FOUR))  followIndex = 4;
    if (IsKeyPressed(KEY_FIVE))  followIndex = 5;
    if (IsKeyPressed(KEY_SIX))   followIndex = 6;
    if (IsKeyPressed(KEY_SEVEN)) followIndex = 7;
    if (IsKeyPressed(KEY_EIGHT)) followIndex = 8;
    if (IsKeyPressed(KEY_NINE))  followIndex = 9;

    // Clamp to valid range
    if (followIndex < 0) followIndex = 0;
    if (followIndex >= (int)bodies.size()) followIndex = 0;

    // --- Orbit controls ---
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) camYaw   -= rotateSpeed * frameTime;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) camYaw   += rotateSpeed * frameTime;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) camPitch += rotateSpeed * frameTime;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) camPitch -= rotateSpeed * frameTime;

    const float pitchLimit = 1.5f;
    if (camPitch >  pitchLimit) camPitch =  pitchLimit;
    if (camPitch < -pitchLimit) camPitch = -pitchLimit;

    // --- Zoom ---
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        camDistance *= (wheel > 0) ? (1.0f / zoomSpeed) : zoomSpeed;
    }
    if (IsKeyDown(KEY_Q)) camDistance *= (1.0f / zoomSpeed) * (1.0f + frameTime);
    if (IsKeyDown(KEY_E)) camDistance *= zoomSpeed * (1.0f + frameTime);

    if (camDistance < 0.1f)  camDistance = 0.1f;
    if (camDistance > 500.0f) camDistance = 500.0f;

    // --- Set camera target to followed body ---
    Vector3 target = toRender(bodies[followIndex].position);
    camera.target = target;

    // --- Position camera relative to target ---
    camera.position.x = target.x + camDistance * cosf(camPitch) * sinf(camYaw);
    camera.position.y = target.y + camDistance * sinf(camPitch);
    camera.position.z = target.z + camDistance * cosf(camPitch) * cosf(camYaw);
}