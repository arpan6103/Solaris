#pragma once
#include "Body.h"
#include<vector>

extern const double G;

std::vector<Vec3>computeAccelerations(const std::vector<Body>&bodies);
void step(std::vector<Body>&bodies,double dt);