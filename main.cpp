#include "Vec3.h"
#include "Body.h"
#include "raylib.h"
#include <vector>
#include<iostream>

constexpr double G  = 6.67430e-11;
const     double AU = 1.495978707e11;
constexpr double SCALE = 2.0 / 1.495978707e11;

Vector3 toRender(const Vec3& p) {
    return {
        static_cast<float>(p.x * SCALE),
        static_cast<float>(p.z * SCALE),
        static_cast<float>(p.y * SCALE)
    };
}

std::vector<Vec3> computeAccelerations(const std::vector<Body>& bodies) {
    std::vector<Vec3> accelerations(bodies.size(), Vec3(0, 0, 0));
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            Vec3 r = bodies[j].position - bodies[i].position;
            double distSq = r.magnitudeSquared();
            double dist   = std::sqrt(distSq);
            double forceMag = G * bodies[i].mass * bodies[j].mass / (distSq * dist);
            Vec3   force    = r * forceMag;
            accelerations[i] += force / bodies[i].mass;
            accelerations[j] -= force / bodies[j].mass;
        }
    }
    return accelerations;
}

void step(std::vector<Body>& bodies, double dt) {
    auto accels = computeAccelerations(bodies);
    for (size_t i = 0; i < bodies.size(); ++i) {
        bodies[i].velocity += accels[i] * dt;
        bodies[i].position += bodies[i].velocity * dt;
    }
}

int main() {
    const double SUN_MASS = 1.989e30;

    struct PlanetDef {
        const char* name;
        double mass;          // kg
        double orbitRadius;   // AU
        double orbitVel;      // m/s
        float  renderRadius;  // raylib units, aesthetic only
        Color  bodyColor;
        Color  trailColor;
    };

    PlanetDef planetDefs[] = {
        // name      mass         radius    vel       render  body         trail
        {"Mercury",  3.3011e23,   0.387,    47360.0,  0.04f,  LIGHTGRAY,   GRAY},
        {"Venus",    4.8675e24,   0.723,    35020.0,  0.07f,  BEIGE,       BROWN},
        {"Earth",    5.972e24,    1.000,    29784.8,  0.07f,  BLUE,        SKYBLUE},
        {"Mars",     6.4171e23,   1.524,    24070.0,  0.05f,  RED,         MAROON},
        {"Jupiter",  1.898e27,    5.203,    13070.0,  0.18f,  ORANGE,      BROWN},
        {"Saturn",   5.683e26,    9.537,     9680.0,  0.15f,  GOLD,        BEIGE},
        {"Uranus",   8.681e25,   19.191,     6800.0,  0.10f,  SKYBLUE,     BLUE},
        {"Neptune",  1.024e26,   30.069,     5430.0,  0.10f,  DARKBLUE,    BLUE},
    };

    std::vector<Body> bodies;

    // Sun at origin, not moving
    bodies.emplace_back(Vec3(0, 0, 0), Vec3(0, 0, 0), SUN_MASS);

    // Each planet starts on +x axis, moving in +y direction
    for (const auto& def : planetDefs) {
        Vec3 pos(def.orbitRadius * AU, 0, 0);
        Vec3 vel(0, def.orbitVel, 0);
        bodies.emplace_back(pos, vel, def.mass);
    }
    Vec3 totalMomentum(0, 0, 0);
    double totalMass=0.0;
    for (const auto& b : bodies) {
        totalMomentum += b.velocity * b.mass;
        totalMass+=b.mass;
    }
    Vec3 comVelocity=totalMomentum/totalMass;
    for(auto& b:bodies){
        b.velocity-=comVelocity;
    }

    const float SUN_RADIUS = 0.15f;

    // Time scale (mutable): simulated seconds per rendered frame.
    // 86400 = 1 day per frame = 60 days/sec at 60 FPS.
    double simSecondsPerFrame = 86400.0;
    const int physicsStepsPerFrame = 24;
    bool paused = false;

    InitWindow(1200, 800, "Gravity Sim");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position   = { 0.0f, 60.0f, 60.0f };
    camera.target     = { 0.0f, 0.0f, 0.0f };
    camera.up         = { 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    double simulatedSeconds = 0.0;

    while (!WindowShouldClose()) {
        // Camera control
        // ---- Custom camera controller ----
        // Camera orbits around the target using spherical coordinates.
        // Arrow keys (or WASD) rotate; mouse wheel / trackpad scrolls zoom.
        static float camYaw      = 0.0f;        // horizontal angle (radians)
        static float camPitch    = 0.7f;        // vertical angle (radians, up from horizontal)
        static float camDistance = 85.0f;       // distance from target

        const float rotateSpeed = 1.5f;         // radians per second
        const float zoomSpeed   = 1.1f;         // multiplicative zoom per scroll tick
        float frameTime = GetFrameTime();

        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) camYaw   -= rotateSpeed * frameTime;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) camYaw   += rotateSpeed * frameTime;
        if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) camPitch += rotateSpeed * frameTime;
        if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) camPitch -= rotateSpeed * frameTime;

        // Clamp pitch so the camera can't flip upside-down
        const float pitchLimit = 1.5f;          // ~85 degrees
        if (camPitch >  pitchLimit) camPitch =  pitchLimit;
        if (camPitch < -pitchLimit) camPitch = -pitchLimit;

        // Trackpad / wheel zoom
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            camDistance *= (wheel > 0) ? (1.0f / zoomSpeed) : zoomSpeed;
        }
        // Keyboard zoom fallback (in case scroll doesn't work)
        if (IsKeyDown(KEY_Q)) camDistance *= (1.0f / zoomSpeed) * (1.0f + frameTime);
        if (IsKeyDown(KEY_E)) camDistance *= zoomSpeed * (1.0f + frameTime);

        // Clamp distance
        if (camDistance < 1.0f)   camDistance = 1.0f;
        if (camDistance > 500.0f) camDistance = 500.0f;

        // Convert spherical (yaw, pitch, distance) to camera position
        camera.position.x = camDistance * cosf(camPitch) * sinf(camYaw);
        camera.position.y = camDistance * sinf(camPitch);
        camera.position.z = camDistance * cosf(camPitch) * cosf(camYaw);
        camera.target = { 0.0f, 0.0f, 0.0f };

        // Time controls
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_KP_ADD)) {
            simSecondsPerFrame *= 1.05;
        }
        if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_KP_SUBTRACT)) {
            simSecondsPerFrame /= 1.05;
        }
        if (IsKeyPressed(KEY_ZERO)) {
            simSecondsPerFrame = 86400.0;
        }

        // Physics
        if (!paused) {
            double dt = simSecondsPerFrame / physicsStepsPerFrame;
            for (int i = 0; i < physicsStepsPerFrame; ++i) {
                step(bodies, dt);
                simulatedSeconds += dt;
            }
            for (auto& body : bodies) {
                body.recordTrail();
            }
        }

        // Render
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                DrawGrid(60, 2.0f);

                for (size_t b = 0; b < bodies.size(); ++b) {
                    Color trailColor;
                    Color bodyColor;
                    float radius;

                    if (b == 0) {
                        trailColor = ORANGE;
                        bodyColor  = YELLOW;
                        radius     = SUN_RADIUS;
                    } else {
                        const auto& def = planetDefs[b - 1];
                        trailColor = def.trailColor;
                        bodyColor  = def.bodyColor;
                        radius     = def.renderRadius;
                    }

                    const auto& trail = bodies[b].trail;
                    for (size_t i = 1; i < trail.size(); ++i) {
                        float t = static_cast<float>(i) / static_cast<float>(trail.size());
                        unsigned char alpha = static_cast<unsigned char>(t * 255);
                        Color faded = { trailColor.r, trailColor.g, trailColor.b, alpha };
                        DrawLine3D(toRender(trail[i - 1]),
                                   toRender(trail[i]),
                                   faded);
                    }

                    DrawSphere(toRender(bodies[b].position), radius, bodyColor);
                }
            EndMode3D();
            for (size_t b = 1; b < bodies.size(); ++b) {
                const auto& def = planetDefs[b - 1];
                Vector3 worldPos = toRender(bodies[b].position);

                // Project the 3D world position to 2D screen coordinates
                Vector2 screenPos = GetWorldToScreen(worldPos, camera);

                // Skip drawing if it's behind the camera (raylib returns weird coords)
                // A quick check: behind-camera positions usually end up offscreen.
                if (screenPos.x > -100 && screenPos.x < GetScreenWidth()  + 100 &&
                    screenPos.y > -100 && screenPos.y < GetScreenHeight() + 100) {
                    // Offset the label slightly above-right of the planet
                    DrawText(def.name,
                             (int)screenPos.x + 8,
                             (int)screenPos.y - 8,
                             14,
                             def.bodyColor);
                }
            }
            // Title and time
            DrawText("Gravity Sim", 10, 10, 20, RAYWHITE);
            DrawText(TextFormat("Simulated days: %.1f", simulatedSeconds / 86400.0),
                     10, 35, 18, RAYWHITE);

            // Time scale + pause status
            double daysPerSec = (simSecondsPerFrame * 60.0) / 86400.0;
            DrawText(TextFormat("Time scale: %.1f days/sec %s",
                                daysPerSec, paused ? "[PAUSED]" : ""),
                     10, 60, 18, paused ? RED : LIME);

            // Per-planet readout
            int textY = 95;
            for (size_t b = 1; b < bodies.size(); ++b) {
                const auto& def = planetDefs[b - 1];
                const Vec3& p = bodies[b].position;
                const Vec3& v = bodies[b].velocity;
                double dist  = p.magnitude() / AU;
                double speed = v.magnitude() / 1000.0;

                DrawText(TextFormat("%-8s  %.3f AU   %.2f km/s",
                                    def.name, dist, speed),
                         10, textY, 16, def.bodyColor);
                textY += 22;
            }

            DrawText("Arrows/WASD: orbit  Q/E or scroll: zoom  Space: pause  +/-: time  0: reset",
                     10, textY + 10, 13, GRAY);
            DrawFPS(10, textY + 35);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}