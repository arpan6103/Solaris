#pragma once 
#include "Vec3.h"
#include<deque>

struct Body{
    Vec3 position;
    Vec3 velocity;
    double mass;

    std::deque<Vec3>trail;
    size_t maxTrailLength=250;

    Body(Vec3 pos, Vec3 vel, double m)
        :position(pos),velocity(vel),mass(m){}

    void recordTrail(){
        trail.push_back(position);
        if(trail.size()>maxTrailLength){
            trail.pop_front();
        }
    }
};