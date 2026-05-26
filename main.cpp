#include "Vec3.h"
#include <iostream>

int main() {
    Vec3 a(1.0, 2.0, 3.0);
    Vec3 b(4.0, 5.0, 6.0);

    Vec3 sum  = a + b;
    Vec3 diff = b - a;
    Vec3 scaled = a * 2.0;

    std::cout << "a + b      = (" << sum.x  << ", " << sum.y  << ", " << sum.z  << ")\n";
    std::cout << "b - a      = (" << diff.x << ", " << diff.y << ", " << diff.z << ")\n";
    std::cout << "a * 2      = (" << scaled.x << ", " << scaled.y << ", " << scaled.z << ")\n";
    std::cout << "|a|        = " << a.magnitude() << "\n";
    std::cout << "|a|^2      = " << a.magnitudeSquared() << "\n";

    return 0;
}