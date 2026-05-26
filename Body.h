#pragma once 
#include "Vec3.h"

struct Body{
    Vec3 position;
    Vec3 velocity;
    double mass;

    Body(Vec3 pos, Vec3 vel, double m)
        :position(pos),velocity(vel),mass(m){}
};