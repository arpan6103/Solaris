#include "Vec3.h"
#include "Body.h"
#include "raylib.h"
#include <vector>

constexpr double G  = 6.67430e-11;
const     double AU = 1.495978707e11;
constexpr double SCALE = 2.0 / 1.495978707e11;

Vector3 toRender(const Vec3& p) {
    return {
        static_cast<float>(p.x * SCALE),
        static_cast<float>(p.y * SCALE),
        static_cast<float>(p.z * SCALE)
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
    const double SUN_MASS        = 1.989e30;
    const double EARTH_MASS      = 5.972e24;
    const double EARTH_ORBIT_VEL = 29784.8;

    std::vector<Body> bodies;
    bodies.emplace_back(Vec3(0,  0, 0),  Vec3(0, 0, 0),                SUN_MASS);
    bodies.emplace_back(Vec3(AU, 0, 0),  Vec3(0, EARTH_ORBIT_VEL, 0),  EARTH_MASS);

    const float SUN_RADIUS   = 0.2f;
    const float EARTH_RADIUS = 0.08f;

    const double SIM_SECONDS_PER_FRAME   = 86400.0;
    const int    PHYSICS_STEPS_PER_FRAME = 24;
    const double DT = SIM_SECONDS_PER_FRAME / PHYSICS_STEPS_PER_FRAME;

    InitWindow(1200, 800, "Gravity Sim");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position   = { 0.0f, 5.0f, 5.0f };
    camera.target     = { 0.0f, 0.0f, 0.0f };
    camera.up         = { 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    double simulatedSeconds = 0.0;

    while (!WindowShouldClose()) {
        //UpdateCamera(&camera,CAMERA_THIRD_PERSON);
        // Physics
        for (int i = 0; i < PHYSICS_STEPS_PER_FRAME; ++i) {
            step(bodies, DT);
            simulatedSeconds += DT;
        }

        // Trails (once per frame)
        for (auto& body : bodies) {
            body.recordTrail();
        }

        // Render
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                DrawGrid(10, 1.0f);

                for (size_t b = 0; b < bodies.size(); ++b) {
                    Color trailColor = (b == 0) ? ORANGE : SKYBLUE;
                    Color bodyColor  = (b == 0) ? YELLOW : BLUE;
                    float radius     = (b == 0) ? SUN_RADIUS : EARTH_RADIUS;

                    const auto& trail = bodies[b].trail;
                    for (size_t i = 1; i < trail.size(); ++i) {
                        float t = static_cast<float>(i) / static_cast<float>(trail.size());
                        unsigned char alpha = static_cast<unsigned char>(t*t*255);
                        Color faded = { trailColor.r, trailColor.g, trailColor.b, alpha };
                        DrawLine3D(toRender(trail[i - 1]),
                                   toRender(trail[i]),
                                   faded);
                    }

                    DrawSphere(toRender(bodies[b].position), radius, bodyColor);
                }
            EndMode3D();

            DrawText("Gravity Sim", 10, 10, 20, RAYWHITE);
            DrawText(TextFormat("Simulated days: %.1f", simulatedSeconds / 86400.0),
                     10, 40, 18, RAYWHITE);
            const Vec3& earthPos=bodies[1].position;
            const Vec3& earthVel=bodies[1].velocity;
            double earthDist=earthPos.magnitude();
            double earthSpeed=earthVel.magnitude();
                        DrawText("Earth:", 10, 80, 18, SKYBLUE);
            DrawText(TextFormat("  Distance: %.4f AU", earthDist / AU),
                     10, 105, 18, RAYWHITE);
            DrawText(TextFormat("  Speed:    %.1f m/s  (%.2f km/s)",
                                earthSpeed, earthSpeed / 1000.0),
                     10, 130, 18, RAYWHITE);
            DrawFPS(10, 165);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}