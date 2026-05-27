#pragma once
#include "raylib.h"
#include "Body.h"
#include <vector>

void updateCamera(Camera3D& camera, const std::vector<Body>& bodies, int& followIndex);