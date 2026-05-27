#include "Physics.h"
#include <cmath>

const double G = 6.67430e-11;

std::vector<Vec3> computeAccelerations(const std::vector<Body>& bodies) {
    std::vector<Vec3> accelerations(bodies.size(), Vec3(0, 0, 0));
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            if (bodies[i].isAsteroid && bodies[j].isAsteroid) continue;
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