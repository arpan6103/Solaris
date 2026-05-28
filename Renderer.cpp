#include "Renderer.h"
#include <cmath>

struct Star{
    Vector3 position;
    Color color;
};
static std::vector<Star>stars;

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
        drawStars(camera.position);
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
                // Star A — yellow
                bodyColor  = YELLOW;
                trailColor = ORANGE;
                radius     = 0.2f;
            } else if (b == 1) {
                // Star B — orange-white
                bodyColor  = { 255, 200, 100, 255 };
                trailColor = { 200, 120,  50, 255 };
                radius     = 0.2f;
            } else if (b - 2 < numPlanets) {
                // Planets
                const auto& def = planetDefs[b - 2];
                trailColor = def.trailColor;
                bodyColor  = def.bodyColor;
                radius     = def.renderRadius;
            } else {
                // Comet — icy blue-white, small
                bodyColor  = { 180, 220, 255, 255 };  // ice blue
                trailColor = { 150, 200, 255, 255 };  // lighter blue trail
                radius     = 0.03f;
            }

            const auto& trail = bodies[b].trail;
            for (size_t i = 1; i < trail.size(); ++i) {
                float t = static_cast<float>(i) / static_cast<float>(trail.size());
                unsigned char alpha = static_cast<unsigned char>(t * 255);
                Color faded = { trailColor.r, trailColor.g, trailColor.b, alpha };
                DrawLine3D(toRender(trail[i - 1]), toRender(trail[i]), faded);
            }
            DrawSphere(toRender(bodies[b].position), radius, bodyColor);
            if (b == 7) {
                drawSaturnRings(bodies[b], camera);
            }
        }
    EndMode3D();

    // Planet labels (3D → 2D projection)
    for (size_t b = 2; b < bodies.size(); ++b) {
        if (bodies[b].isAsteroid) continue;
        if (b - 2 >= numPlanets) continue;
        const auto& def = planetDefs[b - 2];
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
    bool paused,
    int followIndex)
{
    const char* followName;
    Color followColor;
    if      (followIndex == 0) { followName = "Star A"; followColor = YELLOW; }
    else if (followIndex == 1) { followName = "Star B"; followColor = ORANGE; }
    else { followName = planetDefs[followIndex - 2].name;
           followColor = planetDefs[followIndex - 2].bodyColor; }

    DrawText(TextFormat("Following: %s  (0-8 to change)", followName),
             10, 10, 16, followColor);
    DrawText(TextFormat("Simulated days: %.1f", simulatedSeconds / 86400.0),
             10, 35, 18, RAYWHITE);

    double daysPerSec = (simSecondsPerFrame * 60.0) / 86400.0;
    DrawText(TextFormat("Time scale: %.1f days/sec %s",
                        daysPerSec, paused ? "[PAUSED]" : ""),
             10, 60, 18, paused ? RED : LIME);

    int textY = 95;
    for (size_t b = 2; b <= numPlanets+1; ++b) {
        const auto& def = planetDefs[b - 2];
        const Vec3& p = bodies[b].position;
        const Vec3& v = bodies[b].velocity;
        double dist  = p.magnitude() / AU;
        double speed = v.magnitude() / 1000.0;
        DrawText(TextFormat("%-8s  %.3f AU   %.2f km/s",
                            def.name, dist, speed),
                 10, textY, 16, def.bodyColor);
        textY += 22;
    }

    DrawText("Arrows/WASD: orbit  Q/E or scroll: zoom  Space: pause  +/-: time  R: reset 0-1: stars time 2-9: planets",
             10, textY + 10, 13, GRAY);
    DrawFPS(10, textY + 35);
}

void drawSaturnRings(const Body& saturn, const Camera3D& camera) {
    const float RING_INNER_R = 0.20f;   // just outside Saturn's sphere
    const float RING_OUTER_R = 0.45f;   // outer edge
    const int   RING_POINTS  = 800;
    const int   RING_BANDS   = 6;
    const float TILT         = 0.466f;  // 26.7 degrees

    Vector3 satPos = toRender(saturn.position);

    for (int band = 0; band < RING_BANDS; ++band) {
        float t = (float)band / (float)(RING_BANDS - 1);
        float r = RING_INNER_R + t * (RING_OUTER_R - RING_INNER_R);

        unsigned char brightness = (unsigned char)(220 - t * 120);
        Color ringColor = {
            brightness,
            brightness,
            (unsigned char)(brightness * 0.85f),
            200
        };

        for (int p = 0; p < RING_POINTS; ++p) {
            float angle = (2.0f * 3.14159265f * p) / RING_POINTS;

            float lx =  r * cosf(angle);
            float lz =  r * sinf(angle);

            float rx =  lx;
            float ry = -lz * sinf(TILT);
            float rz =  lz * cosf(TILT);

            Vector3 point = {
                satPos.x + rx,
                satPos.y + ry,
                satPos.z + rz
            };

            DrawPoint3D(point, ringColor);
        }
    }
}

void initStars(int count){
    stars.reserve(count);
    for(int i=0;i<count;i++){
        float theta=(float)(rand()/(double)RAND_MAX*2.0f*3.14159265f);
        float phi=acosf(1.0f-2.0f*(float)(rand()/(double)RAND_MAX));
        float radius=350.0f+(float)(rand()/(double)RAND_MAX)*100.0f;

        Star s;
        s.position={
            radius*sinf(phi)*cosf(theta),
            radius*cos(phi),
            radius*sinf(phi)*sinf(theta)
        };
        unsigned char brightness=140+(unsigned char)(rand()%115);
        unsigned char warmth=(unsigned char)(rand()%30);
        s.color={brightness,brightness,(unsigned char)(brightness-warmth),255};
        stars.push_back(s);
    }
}

void drawStars(Vector3 cameraPos){
    for(const auto& s:stars){
        Vector3 pos={
            s.position.x+cameraPos.x,
            s.position.y+cameraPos.y,
            s.position.z+cameraPos.z
        };
        DrawPoint3D(pos,s.color);
    }
}