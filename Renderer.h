#pragma once
#include "Body.h"
#include "raylib.h"
#include <vector>

// Scale factor: physics meters → raylib units
extern const double AU;
extern const double SCALE;

struct PlanetDef {
    const char* name;
    double mass;
    double orbitRadius;
    double orbitVel;
    float  renderRadius;
    Color  bodyColor;
    Color  trailColor;
};

Vector3 toRender(const Vec3& p);

void drawScene(
    const std::vector<Body>& bodies,
    const PlanetDef* planetDefs,
    size_t numPlanets,
    const Camera3D& camera,
    float sunRadius
);

void drawOverlay(
    const std::vector<Body>& bodies,
    const PlanetDef* planetDefs,
    size_t numPlanets,
    double simulatedSeconds,
    double simSecondsPerFrame,
    bool paused,
    int followIndex
);

void drawSaturnRings(const Body& saturn, const Camera3D& camera);