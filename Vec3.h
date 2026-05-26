#pragma once 
#include<cmath>

struct Vec3{
    double x,y,z;
    Vec3(double x=0.0, double y=0.0, double z=0.0):
        x(x),y(y),z(z){}

    Vec3 operator+(const Vec3& o) const {
        return {x+o.x, y+o.y, z+o.z};
    }
    Vec3 operator-(const Vec3& o) const {
        return {x-o.x, y-o.y, z-o.z};
    }
    Vec3 operator*(double s) const {
        return {x*s, y*s, z*s};
    }
    Vec3 operator/(double s) const {
        return {x/s, y/s, z/s};
    }

    Vec3& operator+=(const Vec3 &o){
        x+=o.x;
        y+=o.y;
        z+=o.z;
        return *this;
    }
    Vec3& operator-=(const Vec3 &o){
        x-=o.x;
        y-=o.y;
        z-=o.z;
        return *this;
    }

    double magnitude() const {
        return std::sqrt(x*x+y*y+z*z);
    }
    double magnitudeSquared() const {
        return x*x+y*y+z*z;
    }
};