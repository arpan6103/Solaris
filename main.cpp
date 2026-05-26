#include "Vec3.h"
#include "Body.h"
#include "raylib.h"
#include <vector>

constexpr double G = 6.67430e-11;

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
    // --- Set up the simulation (same as Step 5) ---
    const double AU              = 1.495978707e11;
    const double SUN_MASS        = 1.989e30;
    const double EARTH_MASS      = 5.972e24;
    const double EARTH_ORBIT_VEL = 29784.8;

    std::vector<Body> bodies;
    bodies.emplace_back(Vec3(0,  0, 0),  Vec3(0, 0, 0),                SUN_MASS);
    bodies.emplace_back(Vec3(AU, 0, 0),  Vec3(0, EARTH_ORBIT_VEL, 0),  EARTH_MASS);

    // --- Set up the window ---
    const int screenWidth  = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Gravity Sim");
    SetTargetFPS(60);

    // --- Set up a 3D camera looking down at the scene ---
    Camera3D camera = { 0 };
    camera.position   = { 0.0f, 5.0f, 5.0f };  // where the camera is
    camera.target     = { 0.0f, 0.0f, 0.0f };  // where it's looking
    camera.up         = { 0.0f, 1.0f, 0.0f };  // which way is "up"
    camera.fovy       = 45.0f;                  // field of view (degrees)
    camera.projection = CAMERA_PERSPECTIVE;

    // --- Main loop ---
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                // Reference grid on the y=0 plane, just so we can see in 3D
                DrawGrid(10, 1.0f);

                // Placeholder: a yellow sphere at origin, a blue one to the side.
                // We'll wire these to the bodies in the next step.
                DrawSphere({0.0f, 0.0f, 0.0f}, 0.3f, YELLOW);
                DrawSphere({2.0f, 0.0f, 0.0f}, 0.1f, BLUE);
            EndMode3D();

            DrawText("Gravity Sim - Step 7", 10, 10, 20, RAYWHITE);
            DrawFPS(10, 40);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}