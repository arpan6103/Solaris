#include "Renderer.h"
#include <cmath>

const double AU    = 1.495978707e11;
const double SCALE = 2.0 / 1.495978707e11;

Vector3 toRender(const Vec3& p) {
    return {
        static_cast<float>(p.x * SCALE),
        static_cast<float>(p.z * SCALE),
        static_cast<float>(p.y * SCALE)
    };
}

void drawScene(
    const std::vector<Body>& bodies,
    const PlanetDef* planetDefs,
    size_t numPlanets,
    const Camera3D& camera,
    float sunRadius)
{
    BeginMode3D(camera);
        DrawGrid(60, 2.0f);

        for (size_t b = 0; b < bodies.size(); ++b) {
            if (bodies[b].isAsteroid) {
                DrawPoint3D(toRender(bodies[b].position), GRAY);
                continue;
            }

            Color trailColor;
            Color bodyColor;
            float radius;

            if (b == 0) {
                trailColor = ORANGE;
                bodyColor  = YELLOW;
                radius     = sunRadius;
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
                DrawLine3D(toRender(trail[i - 1]), toRender(trail[i]), faded);
            }
            DrawSphere(toRender(bodies[b].position), radius, bodyColor);
        }
    EndMode3D();

    // Planet labels (3D → 2D projection)
    for (size_t b = 1; b < bodies.size(); ++b) {
        if (bodies[b].isAsteroid) continue;
        const auto& def = planetDefs[b - 1];
        Vector3 worldPos  = toRender(bodies[b].position);
        Vector2 screenPos = GetWorldToScreen(worldPos, camera);
        if (screenPos.x > -100 && screenPos.x < GetScreenWidth()  + 100 &&
            screenPos.y > -100 && screenPos.y < GetScreenHeight() + 100) {
            DrawText(def.name,
                     (int)screenPos.x + 8,
                     (int)screenPos.y - 8,
                     14,
                     def.bodyColor);
        }
    }
}

void drawOverlay(
    const std::vector<Body>& bodies,
    const PlanetDef* planetDefs,
    size_t numPlanets,
    double simulatedSeconds,
    double simSecondsPerFrame,
    bool paused)
{
    DrawText("Gravity Sim", 10, 10, 20, RAYWHITE);
    DrawText(TextFormat("Simulated days: %.1f", simulatedSeconds / 86400.0),
             10, 35, 18, RAYWHITE);

    double daysPerSec = (simSecondsPerFrame * 60.0) / 86400.0;
    DrawText(TextFormat("Time scale: %.1f days/sec %s",
                        daysPerSec, paused ? "[PAUSED]" : ""),
             10, 60, 18, paused ? RED : LIME);

    int textY = 95;
    for (size_t b = 1; b <= numPlanets; ++b) {
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
}