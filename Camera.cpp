#include "Camera.h"
#include <cmath>

void updateCamera(Camera3D& camera) {
    static float camYaw      = 0.0f;
    static float camPitch    = 0.7f;
    static float camDistance = 85.0f;

    const float rotateSpeed = 1.5f;
    const float zoomSpeed   = 1.1f;
    float frameTime = GetFrameTime();

    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) camYaw   -= rotateSpeed * frameTime;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) camYaw   += rotateSpeed * frameTime;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) camPitch += rotateSpeed * frameTime;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) camPitch -= rotateSpeed * frameTime;

    const float pitchLimit = 1.5f;
    if (camPitch >  pitchLimit) camPitch =  pitchLimit;
    if (camPitch < -pitchLimit) camPitch = -pitchLimit;

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        camDistance *= (wheel > 0) ? (1.0f / zoomSpeed) : zoomSpeed;
    }
    if (IsKeyDown(KEY_Q)) camDistance *= (1.0f / zoomSpeed) * (1.0f + frameTime);
    if (IsKeyDown(KEY_E)) camDistance *= zoomSpeed * (1.0f + frameTime);

    if (camDistance < 1.0f)   camDistance = 1.0f;
    if (camDistance > 500.0f) camDistance = 500.0f;

    camera.position.x = camDistance * cosf(camPitch) * sinf(camYaw);
    camera.position.y = camDistance * sinf(camPitch);
    camera.position.z = camDistance * cosf(camPitch) * cosf(camYaw);
    camera.target = { 0.0f, 0.0f, 0.0f };
}