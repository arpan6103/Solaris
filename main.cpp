#include "Vec3.h"
#include "Body.h"
#include "Physics.h"
#include "Renderer.h"
#include "Camera.h"
#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <cmath>

int main() {
    const double SUN_MASS = 1.989e30;

    PlanetDef planetDefs[] = {
        {"Mercury",  3.3011e23,   0.387,    47360.0,  0.04f,  LIGHTGRAY,   GRAY},
        {"Venus",    4.8675e24,   0.723,    35020.0,  0.07f,  BEIGE,       BROWN},
        {"Earth",    5.972e24,    1.000,    29784.8,  0.07f,  BLUE,        SKYBLUE},
        {"Mars",     6.4171e23,   1.524,    24070.0,  0.05f,  RED,         MAROON},
        {"Jupiter",  1.898e27,    5.203,    13070.0,  0.18f,  ORANGE,      BROWN},
        {"Saturn",   5.683e26,    9.537,     9680.0,  0.15f,  GOLD,        BEIGE},
        {"Uranus",   8.681e25,   19.191,     6800.0,  0.10f,  SKYBLUE,     BLUE},
        {"Neptune",  1.024e26,   30.069,     5430.0,  0.10f,  DARKBLUE,    BLUE},
    };
    const size_t NUM_PLANETS = sizeof(planetDefs) / sizeof(planetDefs[0]);

    std::vector<Body> bodies;

    // Sun
   // --- Two stars, each half the Sun's mass, separated by 0.4 AU ---
    const double STAR_MASS = SUN_MASS * 0.5;
    const double HALF_SEP  = 0.2 * AU;

    // Orbital velocity for each star around their COM:
    // v = sqrt(G * M_other / (2 * separation))
    double starVel = std::sqrt(G * STAR_MASS / (2.0 * 2.0 * HALF_SEP));

    // Star A: left, moving in -y
    bodies.emplace_back(
        Vec3(-HALF_SEP, 0, 0),
        Vec3(0, -starVel, 0),
        STAR_MASS
    );

    // Star B: right, moving in +y
    bodies.emplace_back(
        Vec3(HALF_SEP, 0, 0),
        Vec3(0, starVel, 0),
        STAR_MASS
    );

    // Planets — same orbits as before
    for (size_t i = 0; i < NUM_PLANETS; ++i) {
        Vec3 pos(planetDefs[i].orbitRadius * AU, 0, 0);
        Vec3 vel(0, planetDefs[i].orbitVel, 0);
        bodies.emplace_back(pos, vel, planetDefs[i].mass);
    }

    // Cancel COM drift
    Vec3 totalMomentum(0, 0, 0);
    double totalMass = 0.0;
    for (const auto& b : bodies) {
        totalMomentum += b.velocity * b.mass;
        totalMass     += b.mass;
    }
    Vec3 comVelocity = totalMomentum / totalMass;
    for (auto& b : bodies) b.velocity -= comVelocity;

    // Asteroid belt
    const int    NUM_ASTEROIDS = 2000;
    const double BELT_INNER    = 2.2;
    const double BELT_OUTER    = 3.2;
    const double ASTEROID_MASS = 1e15;

    srand(42);
    auto randFloat = [](double lo, double hi) {
        return lo + (hi - lo) * (rand() / (double)RAND_MAX);
    };

    for (int i = 0; i < NUM_ASTEROIDS; ++i) {
        double r     = randFloat(BELT_INNER, BELT_OUTER) * AU;
        double angle = randFloat(0.0, 2.0 * 3.14159265358979);
        double incl  = randFloat(-0.035, 0.035);
        Vec3 pos(
            r * cos(angle),
            r * sin(angle),
            r * sin(incl)
        );
        double speed = std::sqrt(G * SUN_MASS / r);
        Vec3 vel(
            -speed * sin(angle),
             speed * cos(angle),
             0.0
        );
        bodies.emplace_back(pos, vel, ASTEROID_MASS, true);
    }
    // --- Halley-style comet ---
    {
        const double COMET_PERIHELION = 0.6  * AU;   // closest approach
        const double COMET_APHELION   = 35.0 * AU;   // farthest point
        const double COMET_MASS       = 2.2e14;       // kg (tiny)

        double a       = (COMET_PERIHELION + COMET_APHELION) / 2.0;
        double v_apo   = std::sqrt(G * SUN_MASS * (2.0/COMET_APHELION - 1.0/a));

        // Start at aphelion on the -x axis, moving in +y (retrograde — like Halley's)
        Vec3 cometPos(-COMET_APHELION, 0, 0);
        Vec3 cometVel(0, -v_apo, 0);   // negative = retrograde orbit

        bodies.emplace_back(cometPos, cometVel, COMET_MASS);
        bodies.back().maxTrailLength = 3000;
    }
    initStars(2000);

    // Window + camera
    const float SUN_RADIUS = 0.15f;
    double simSecondsPerFrame = 86400.0;
    const int physicsStepsPerFrame = 24;
    bool paused = false;
    double simulatedSeconds = 0.0;
    int followIndex = 0;

    InitWindow(1200, 800, "Gravity Sim");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position   = { 0.0f, 60.0f, 60.0f };
    camera.target     = { 0.0f, 0.0f, 0.0f };
    camera.up         = { 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        // Camera
        updateCamera(camera, bodies, followIndex);

        // Time controls
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyDown(KEY_EQUAL)  || IsKeyDown(KEY_KP_ADD))      simSecondsPerFrame *= 1.05;
        if (IsKeyDown(KEY_MINUS)  || IsKeyDown(KEY_KP_SUBTRACT)) simSecondsPerFrame /= 1.05;
        if (IsKeyPressed(KEY_R)) simSecondsPerFrame = 86400.0;

        // Physics
        if (!paused) {
            double dt = simSecondsPerFrame / physicsStepsPerFrame;
            for (int i = 0; i < physicsStepsPerFrame; ++i) {
                step(bodies, dt);
                simulatedSeconds += dt;
            }
            for (auto& body : bodies) body.recordTrail();
        }

        // Render
        BeginDrawing();
            ClearBackground(BLACK);
            drawScene(bodies, planetDefs, NUM_PLANETS, camera, SUN_RADIUS);
            drawOverlay(bodies, planetDefs, NUM_PLANETS,
                        simulatedSeconds, simSecondsPerFrame, paused,followIndex);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}